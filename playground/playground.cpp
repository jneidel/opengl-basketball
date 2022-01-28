#include <stdio.h>
#include <stdlib.h>

/*
  docs
    - video + commentary
*/

// printing
#include <iostream>
#include <ostream>

// opengl wrapper
#include <GL/glew.h>

#include <GLFW/glfw3.h>

// math
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/ext.hpp" // for print
using namespace glm;

#include "btBulletDynamicsCommon.h"

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/objloader.hpp>
#include <common/controls.hpp>
#include <common/text2D.hpp>

/* utils */
void print_mat4( mat4 toPrint ) {
  // src: https://stackoverflow.com/a/15046613
  std::cout<<glm::to_string(toPrint)<<std::endl;
}
void print_vec4( vec4 toPrint ) {
  std::cout<<glm::to_string(toPrint)<<std::endl;
}

/* physics engine */
void createSurrounding(bool isDepth, bool isFrontOrRight, float width, float depth, float depthOffset, btDiscreteDynamicsWorld* dynamicsWorld, btAlignedObjectArray<btCollisionShape*> collisionShapes) {
  float height = 6.f;
  float depthFromCenter = (depth/2)+depthOffset;
  float widthFromCenter = width/2;

  btCollisionShape* shape;
  btTransform groundTransform;
  groundTransform.setIdentity();
  if ( isDepth ) { // going from front to back
    shape = new btBoxShape(btVector3(btScalar(0.1), btScalar(height), btScalar(depth)));
    if ( isFrontOrRight ) // right
      groundTransform.setOrigin(btVector3(+widthFromCenter, height/2, depthFromCenter));
    else // left
      groundTransform.setOrigin(btVector3(-widthFromCenter, height/2, depthFromCenter));
  } else { // going from left to right
    shape = new btBoxShape(btVector3(btScalar(width), btScalar(height), btScalar(0.1f)));
    if ( isFrontOrRight ) // front
      groundTransform.setOrigin(btVector3(0, height/2, (depth+depthOffset)*-1));
    else // back
      groundTransform.setOrigin(btVector3(0, height/2,      0+(depthOffset*-1)));
  }

  btScalar mass(0.);
  btVector3 localInertia(0, 0, 0);
  btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
  rbInfo.m_restitution = 0.5f;
  rbInfo.m_friction = 30.f;
  btRigidBody* body = new btRigidBody(rbInfo);
  dynamicsWorld->addRigidBody(body);
  collisionShapes.push_back(shape);
}

btVector3 ballPosition = btVector3(0, 3, 0);
btRigidBody* ballBody;
btDefaultMotionState* ballMotionState;
btDiscreteDynamicsWorld* initBulletWorld() {
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -9.82, 0));
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	//the ground is a cube of side 100 at position y = -56.
	//the sphere will hit it at y = -6, with center at -5
  {
    btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));
    collisionShapes.push_back(groundShape);
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0, -50.5, 0));
    btScalar mass(0.);
    btVector3 localInertia(0, 0, 0);
    btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
    rbInfo.m_restitution = 0.5f;
    rbInfo.m_friction = 30.f;
    btRigidBody* body = new btRigidBody(rbInfo);
    dynamicsWorld->addRigidBody(body);
  }
  {
    btCollisionShape* ballShape = new btSphereShape(btScalar(1.));
    collisionShapes.push_back(ballShape);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(ballPosition);

    btScalar mass(1.f);
    btVector3 localInertia(0, 0, 0);
    ballShape->calculateLocalInertia(mass, localInertia);
    ballMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, ballMotionState, ballShape, localInertia);
    rbInfo.m_restitution = 1.7f;
    rbInfo.m_friction = 1.5f;
    ballBody = new btRigidBody(rbInfo);

    /* ballBody->applyForce(btVector3(0, 0, -35), btVector3(0, 6, 12)); */

    dynamicsWorld->addRigidBody(ballBody);
  }
  {
    btCollisionShape* roofShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));
    collisionShapes.push_back(roofShape);
    btTransform roofTransform;
    roofTransform.setIdentity();
    roofTransform.setOrigin(btVector3(0, 60.5, 0));
    btScalar mass(0.);
    btVector3 localInertia(0, 0, 0);
    btDefaultMotionState* myMotionState = new btDefaultMotionState(roofTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, roofShape, localInertia);
    rbInfo.m_restitution = 0.5f;
    rbInfo.m_friction = 30.f;
    btRigidBody* body = new btRigidBody(rbInfo);
    dynamicsWorld->addRigidBody(body);
  }

  { // surroundings
    float depth = 12.f;
    float width = 7.f;
    float depthOffset = -5.f;

    createSurrounding( true,  true,  width, depth, depthOffset, dynamicsWorld, collisionShapes );
    createSurrounding( true,  false, width, depth, depthOffset, dynamicsWorld, collisionShapes );
    createSurrounding( false, true,  width, depth, depthOffset, dynamicsWorld, collisionShapes );
    createSurrounding( false, false, width, depth, depthOffset, dynamicsWorld, collisionShapes );
  }

  return dynamicsWorld;
}
void resetBallPosition() {
  // src: https://stackoverflow.com/a/32180454
  btTransform resetTransform;
  resetTransform.setOrigin(ballPosition);

  ballBody->setWorldTransform(resetTransform);
  ballBody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
  ballBody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
  ballBody->clearForces();
  ballMotionState->setWorldTransform(resetTransform);
}

/* opengl objects */
// mapping the objects from physics engine to opengl wasn't possible
// (I only found examples using glut not glfw)

unsigned int vb_color;

unsigned int va_ground;
unsigned int vb_ground;
void init_ground() {
  float top_left_x  = -9.0;
  float top_left_z  = -15.0;
  float top_right_x =  9.0;
  float bot_left_z  =  100.0;
  float y = 0.95;

  static const GLfloat g_vertex_buffer_data_triange[] = {
    top_left_x,  y, top_left_z,
    top_left_x,  y, bot_left_z,
    top_right_x, y, top_left_z,
    top_right_x, y, top_left_z,
    top_left_x,  y, bot_left_z,
    top_right_x, y, bot_left_z,
  };

  static const GLfloat g_color_buffer_data[] = {
    0.0,    1,  0.0,
      1,  0.0,  0.0,
    0.0,    1,  0.0,
    0.0,    1,  0.0,
      1,  0.0,  0.0,
      1,  0.0,  0.0,
  };

  glGenVertexArrays(1, &va_ground);
  glBindVertexArray(va_ground);
  glGenBuffers(1, &vb_ground);
  glBindBuffer(GL_ARRAY_BUFFER, vb_ground);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_triange), g_vertex_buffer_data_triange, GL_STATIC_DRAW);
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  // color
  glGenBuffers(1, &vb_color);
  glBindBuffer(GL_ARRAY_BUFFER, vb_color);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
  glVertexAttribPointer( 1, // matches the layout in the shader
    3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

float right_x = 6.3;
float left_x = right_x *-1;

float top_y = 24.0;
float bot_y = 0.95;

float front_z = -15.0;
float back_z = 14.0;

unsigned int va_sides;
unsigned int vb_sides;
void init_sides() { // and ground
  static const GLfloat g_vertex_buffer_data_triange[] = {
    left_x,  bot_y, back_z, // back wall
    left_x,  top_y, back_z,
    right_x, bot_y, back_z,
    left_x,  top_y, back_z,
    right_x, top_y, back_z,
    right_x, bot_y, back_z,
    left_x,  bot_y, front_z, // ground
    left_x,  bot_y, back_z,
    right_x, bot_y, front_z,
    right_x, bot_y, front_z,
    left_x,  bot_y, back_z,
    right_x, bot_y, back_z,
    left_x, bot_y, back_z, // left
    left_x, bot_y, front_z,
    left_x, top_y, front_z,
    left_x, bot_y, back_z,
    left_x, top_y, front_z,
    left_x, top_y, back_z,
    left_x,  top_y, front_z, // front
    left_x,  bot_y, front_z,
    right_x, bot_y, front_z,
    right_x, top_y, front_z,
    left_x,  top_y, front_z,
    right_x, bot_y, front_z,
    right_x, bot_y, front_z,
    right_x, bot_y, back_z, // right
    right_x, top_y, front_z,
    right_x, top_y, front_z,
    right_x, bot_y, back_z,
    right_x, top_y, back_z,
  };

  // green: 0.23 0.93 0.67
  // blue : 0.08 0.72 0.89

  // red 0.90 0.11 0.36
  // orange 1.00 0.74 0.22

  static const GLfloat g_color_buffer_data[] = {
    0.23, 0.93, 0.67, // back
    0.23, 0.93, 0.67,
    0.23, 0.93, 0.67,
    0.23, 0.93, 0.67,
    0.23, 0.93, 0.67,
    0.23, 0.93, 0.67,
    0.08, 0.72, 0.89, // ground
    0.23, 0.93, 0.67,
    0.08, 0.72, 0.89,
    0.08, 0.72, 0.89,
    0.23, 0.93, 0.67,
    0.23, 0.93, 0.67,
    1.00, 0.74, 0.22, // left
    1.00, 0.74, 0.22,
    0.90, 0.11, 0.36,
    1.00, 0.74, 0.22,
    0.90, 0.11, 0.36,
    0.90, 0.11, 0.36,
    0.90, 0.11, 0.36, // front
    1.00, 0.74, 0.22,
    1.00, 0.74, 0.22,
    0.90, 0.11, 0.36,
    0.90, 0.11, 0.36,
    1.00, 0.74, 0.22,
    1.00, 0.74, 0.22, // right
    1.00, 0.74, 0.22,
    0.90, 0.11, 0.36,
    0.90, 0.11, 0.36,
    1.00, 0.74, 0.22,
    0.90, 0.11, 0.36,
  };

  glGenVertexArrays(1, &va_sides);
  glBindVertexArray(va_sides);
  glGenBuffers(1, &vb_sides);
  glBindBuffer(GL_ARRAY_BUFFER, vb_sides);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_triange), g_vertex_buffer_data_triange, GL_STATIC_DRAW);
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  // color
  glGenBuffers(1, &vb_color);
  glBindBuffer(GL_ARRAY_BUFFER, vb_color);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
  glVertexAttribPointer( 1, // matches the layout in the shader
    3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

unsigned int va_roof;
unsigned int vb_roof;
void init_roof() {
  static const GLfloat g_vertex_buffer_data_triange[] = {
    left_x,  top_y, front_z,
    right_x, top_y, front_z,
    left_x,  top_y, back_z,
    right_x, top_y, front_z,
    right_x, top_y, back_z,
    left_x,  top_y, back_z,
  };

  static const GLfloat g_color_buffer_data[] = {
      1,  0.9,  0.0,
      1,  0.9,  0.0,
      1,  0.9,  0.0,
      1,  0.9,  0.0,
      1,  0.9,  0.0,
      1,  0.9,  0.0,
  };

  glGenVertexArrays(1, &va_roof);
  glBindVertexArray(va_roof);
  glGenBuffers(1, &vb_roof);
  glBindBuffer(GL_ARRAY_BUFFER, vb_roof);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_triange), g_vertex_buffer_data_triange, GL_STATIC_DRAW);
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  // color
  glGenBuffers(1, &vb_color);
  glBindBuffer(GL_ARRAY_BUFFER, vb_color);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
  glVertexAttribPointer( 1, // matches the layout in the shader
    3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

unsigned int va_target;
unsigned int vb_target;
void init_target() {
  float z = front_z+0.01;

  float target_dimension = 0.75;
  top_y = 3;
  left_x = -target_dimension/2;
  float mid_x = 0;
  right_x = left_x *-1;
  float mid_y = top_y-target_dimension/2;
  bot_y = top_y-target_dimension;

  float target_bar_width = 0.05;

  static const GLfloat g_vertex_buffer_data_triange[] = {
    left_x, mid_y, z,
    right_x, mid_y, z,
    left_x, mid_y+target_bar_width, z,
    right_x, mid_y, z,
    right_x, mid_y+target_bar_width, z,
    left_x, mid_y+target_bar_width, z,
    mid_x, top_y, z,
    mid_x, bot_y, z,
    mid_x+target_bar_width, top_y, z,
    mid_x+target_bar_width, top_y, z,
    mid_x, bot_y, z,
    mid_x+target_bar_width, bot_y, z,
  };

  static const GLfloat g_color_buffer_data[] = {
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
    1,  0.0,  0.0,
  };

  glGenVertexArrays(1, &va_target);
  glBindVertexArray(va_target);
  glGenBuffers(1, &vb_target);
  glBindBuffer(GL_ARRAY_BUFFER, vb_target);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_triange), g_vertex_buffer_data_triange, GL_STATIC_DRAW);
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  // color
  glGenBuffers(1, &vb_color);
  glBindBuffer(GL_ARRAY_BUFFER, vb_color);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
  glVertexAttribPointer( 1, // matches the layout in the shader
    3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

unsigned int va_ball;
unsigned int vb_ball;
unsigned int va_ball_tex;
unsigned int vb_ball_tex;
unsigned int ball_vertices_size;
void init_ball(GLuint programID) {
  std::vector< glm::vec3 > vertices;
  std::vector< glm::vec2 > uvs;
  std::vector< glm::vec3 > normals; // Unused
  bool res = loadOBJ("../playground/ball.obj", vertices, uvs, normals);
  ball_vertices_size = vertices.size();

  glGenVertexArrays(1, &va_ball);
  glBindVertexArray(va_ball);

  glGenBuffers(1, &vb_ball);
  glBindBuffer(GL_ARRAY_BUFFER, vb_ball);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer( 0, // matches the layout in the shader
    3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	va_ball_tex = loadDDS("../playground/ball.dds"); // glBindTexture(GL_TEXTURE_2D, va_ball_tex);
  glGenBuffers(1, &vb_ball_tex);
  glBindBuffer(GL_ARRAY_BUFFER, vb_ball_tex);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
  glVertexAttribPointer( 1, // matches the layout in the shader
    2, // <- note the 2
    GL_FLOAT, GL_FALSE, 0, (void*)0 );

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

/* configure ball */
float rotation_ball = 0;
float rotate_ball() {
  rotation_ball = rotation_ball +1 % 360;
  return rotation_ball;
}

float force = 50;
void fireBall(GLFWwindow* window, int button, int action, int mods) {
  // propell ball roughly in the direction you're looking
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    /* For the angles from glfw:
     * 6.28 is 360°
     * 3.14 is 180°
     * 1.57 is  90°
     */
    float h = (getHorizontalAngle()-3.14)*-1; // shifted by 180° and inverted axis
    float v = getVerticalAngle();
    if (h < -1.57 || h > 1.57) {
      fprintf( stderr, "Camera looking out of bounds, won't fire (h=%f)\n", h );
      return;
    }
    if (v < -1.57 || v > 1.57) {
      fprintf( stderr, "Camera looking out of bounds, won't fire (v=%f)\n", h );
      return;
    }

    /* Force calcs (my own formula)
     *   example: x all the way to the left (-1.57) and y=0
     *       xForce = -1
     *       yForce =  0
     *       zForce = (1-1)*(1-0) = 0
     *
     *   Another one: x=1 y=1 (up to the right)
     *       xForce = 0.63
     *       yForce = 0.63
     *       zForce = (1-0.63)*(1-0.63) = 0.13
     */

    float xForce = h/1.57;
    float yForce = v/1.57;
    float zForce = (1-abs(xForce))*(1-abs(yForce))*-1; // inverted axis

    ballBody->applyForce(
      btVector3(xForce*force, yForce*force, zForce*force),
      btVector3(0,6,12)
    );
  }
  return;
}

int main( void ) {
  // Initialise GLFW
  glewExperimental = true;
  if( !glfwInit() ) {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow( 1024, 720, "Playground", NULL, NULL );
  if ( window == NULL ) {
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetMouseButtonCallback(window, fireBall);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Initialize GLEW
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwTerminate();
    return -1;
  }


  glEnable( GL_DEPTH_TEST ); // Z-Buffer
  glDepthFunc( GL_LESS ); // accept fragments if they are closer to the camera
	glEnable(GL_CULL_FACE); // backface culling

  glClearColor(0.8f, 0.0f, 0.4f, 0.0f);

  // shaders
  GLuint programID_Color = LoadShaders( "../common/Simple.vertexshader", "../common/Color.fragmentshader" );
  GLuint programID_Texture = LoadShaders( "../common/Transform.vertexshader", "../common/Texture.fragmentshader" );

  // MVP data
  mat4 Projection = perspective( radians(45.0f),  4.f/3.f, 0.1f, 100.0f ); // 45° field of view, 4:3, display range: 0.1 unit <-> 100 units
  mat4 View = lookAt(
    vec3(0, 6, 12), // positon of the camera in world space
    vec3(0, 1, 0), // looks at the origin
    vec3(0, 1, 0)  // head is up, -1 to look upside down
  );
  mat4 scale2x = scale(mat4(), vec3(2.5,2.5,2.5));

  // initialize object buffers
  init_ball(programID_Texture);
  init_sides();
  init_roof();
  init_target();
  initText2D( "../playground/Holstein.dds" );

  btDiscreteDynamicsWorld* dynamicsWorld = initBulletWorld();

  unsigned int hitCounter = 0;
  unsigned int ticks = 0;
  unsigned int tickOnLastHit = 0;

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // keys: https://www.glfw.org/docs/latest/group__keys.html
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
      printf("ENTER -> reset\n");
      resetBallPosition();
    }

    // MVP calc
    computeMatricesFromInputs(window);
    Projection = getProjectionMatrix();
    View = getViewMatrix();
    mat4 MVP_sides = Projection * View * mat4();

    // move ball in physics engine
    dynamicsWorld->stepSimulation(1.f / 60.f, 10);
    btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1];
    btRigidBody* body = btRigidBody::upcast(obj);
    btTransform trans;
    trans = obj->getWorldTransform();

    // translate ball position to opengl
    const float ballX = trans.getOrigin().getX();
    const float ballY = trans.getOrigin().getY();
    const float ballZ = trans.getOrigin().getZ();
    mat4 matchBallWithSim = translate(mat4(), vec3(ballX, ballY, ballZ));
    // printf("x=%.02f, y=%.02f, z=%.02f\n", ballX, ballY, ballZ );

    // register hits on the target
    ticks += 1;
    unsigned int tickDifference = ticks - tickOnLastHit;
    const int HIT_TIMEOUT = 15; // 15 ticks
    // don't trigger the counter more than once per enting of the hit zone

    const float HIT_TOLERANCE = 0.22;
    const float BACK_WALL_Z = -5.90;
    const float MIDDLE_X = 0.0;
    const float MIDDLE_Y = 1.20;
    if ( tickDifference > HIT_TIMEOUT &&
        ballZ <= BACK_WALL_Z + HIT_TOLERANCE &&
        ballX >= MIDDLE_X - HIT_TOLERANCE && ballX <= MIDDLE_X + HIT_TOLERANCE &&
        ballY >= MIDDLE_Y - HIT_TOLERANCE && ballY <= MIDDLE_Y + HIT_TOLERANCE ) {
      hitCounter += 1;
      tickOnLastHit = ticks;
    }

    /* draw objects */
    { // ball
      const float FLOOR_Y = 0.50;
      if ( ballY > FLOOR_Y ) {
        rotate_ball(); // looks cooler if the ball is spinning ;)
      }

      mat4 ballRotation = rotate(mat4(), radians((float) rotation_ball), vec3(1, 1, 0) );
      mat4 Model = mat4(1) * scale2x * matchBallWithSim * ballRotation;
      mat4 MVP = Projection * View * Model;

      glUseProgram(programID_Texture);
      GLuint MatrixID = glGetUniformLocation(programID_Texture, "MVP");

      glUniformMatrix4fv( MatrixID, 1, GL_FALSE, &MVP[0][0] );
      glBindVertexArray(va_ball);
      glBindBuffer(GL_ARRAY_BUFFER, vb_ball);
      glBindTexture(GL_TEXTURE_2D, va_ball_tex);
      glBindBuffer(GL_ARRAY_BUFFER, vb_ball_tex);
      glDrawArrays(GL_TRIANGLES, 0, ball_vertices_size);
    }
    { // sides
      glUseProgram(programID_Color);
      GLuint MatrixID_Color = glGetUniformLocation(programID_Color, "MVP");

      glUniformMatrix4fv( MatrixID_Color, 1, GL_FALSE, &MVP_sides[0][0] );
      glBindVertexArray(va_sides);
      glBindBuffer(GL_ARRAY_BUFFER, vb_sides);
      glDrawArrays(GL_TRIANGLES, 0, 3*10); // each triange is 3
    }
    { // roof
      glUseProgram(programID_Color);
      GLuint MatrixID_Color = glGetUniformLocation(programID_Color, "MVP");

      glUniformMatrix4fv( MatrixID_Color, 1, GL_FALSE, &MVP_sides[0][0] );
      glBindVertexArray(va_roof);
      glBindBuffer(GL_ARRAY_BUFFER, vb_roof);
      glDrawArrays(GL_TRIANGLES, 0, 3*2); // each triange is 3
    }
    { // score text
      // somehow overwrites roof color
      char text[256];
      sprintf(text,"Score: %d", hitCounter );
      printText2D(text, 10, 560, 30);
    }
    { // target
      glUseProgram(programID_Color);
      GLuint MatrixID_Color = glGetUniformLocation(programID_Color, "MVP");

      glUniformMatrix4fv( MatrixID_Color, 1, GL_FALSE, &MVP_sides[0][0] );
      glBindVertexArray(va_target);
      glBindBuffer(GL_ARRAY_BUFFER, vb_target);
      glDrawArrays(GL_TRIANGLES, 0, 3*4); // each triange is 3
    }

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
  } while(
    glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
    glfwWindowShouldClose(window) == 0 &&
    glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS
  );

  // cleanup
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDeleteProgram(programID_Color);
  glDeleteProgram(programID_Texture);
  cleanupText2D();

  glfwTerminate();
  return 0;
}
