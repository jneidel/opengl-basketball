#include <stdio.h>
#include <stdlib.h>

/* MVP Specs

  - shooting
    - load up force with left mouse click
    - bar to display force
    - ball rotation
      - func calculating rotation based on last location (differences)
    - stop sliding on forever (friction?)
  - scene
    - hoop
      - working hit box
      - register korb (sucess)
    - board
    - pole
  - docs
    - readme
    - how to play
      - keybindings
    - video

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

void print_mat4( mat4 toPrint ) {
  // src: https://stackoverflow.com/a/15046613
  std::cout<<glm::to_string(toPrint)<<std::endl;
}
void print_vec4( vec4 toPrint ) {
  std::cout<<glm::to_string(toPrint)<<std::endl;
}

float hoopOffset = 1.0;
btVector3 backLocation(0, 0, hoopOffset*-1);
btVector3 frontLocation(0, 0, hoopOffset);
btVector3 rightLocation(hoopOffset, 0, 0);
btVector3 leftLocation(hoopOffset*-1, 0, 0);
vec2 hoopHitboxX;
float hoopHitboxY;
vec2 hoopHitboxZ;

mat4 hoopTranslation = mat4();
void hoop_location(float x, float y, float z) {
  backLocation = btVector3(x, y, z+hoopOffset*-1);
  frontLocation = btVector3(x, y, z+hoopOffset);
  rightLocation = btVector3(x+hoopOffset, y, z);
  leftLocation = btVector3(x+hoopOffset*-1, y, z);
  hoopTranslation = translate(mat4(), vec3(x, y, z));

  hoopHitboxX = vec2( x+hoopOffset*-1, x+hoopOffset );
  hoopHitboxY = y;
  hoopHitboxZ = vec2( z+hoopOffset*-1, z+hoopOffset );
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

  btVector3 hoopShape(0.01, 0.01, 0.01);
  float hoopRestitution = 0.5f;
  float hoopFriction = 1.0f;

  {
		btCollisionShape* hoop_back = new btBoxShape(hoopShape);
		collisionShapes.push_back(hoop_back);

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(backLocation);

		btScalar mass(0.);
		btVector3 localInertia(0, 0, 0);
		btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, hoop_back, localInertia);
    rbInfo.m_restitution = hoopRestitution;
    rbInfo.m_friction = hoopFriction;
		btRigidBody* body = new btRigidBody(rbInfo);
		dynamicsWorld->addRigidBody(body);
	}
  {
		btCollisionShape* hoop_front = new btBoxShape(hoopShape);
		collisionShapes.push_back(hoop_front);
		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(frontLocation);
		btScalar mass(0.);
		btVector3 localInertia(0, 0, 0);
		btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, hoop_front, localInertia);
    rbInfo.m_restitution = hoopRestitution;
    rbInfo.m_friction = hoopFriction;
		btRigidBody* body = new btRigidBody(rbInfo);
		dynamicsWorld->addRigidBody(body);
	}
  {
		btCollisionShape* hoop_right = new btBoxShape(hoopShape);
		collisionShapes.push_back(hoop_right);

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(rightLocation);

		btScalar mass(0.);
		btVector3 localInertia(0, 0, 0);
		btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, hoop_right, localInertia);
    rbInfo.m_restitution = hoopRestitution;
    rbInfo.m_friction = hoopFriction;
		btRigidBody* body = new btRigidBody(rbInfo);
		dynamicsWorld->addRigidBody(body);
	}
  {
		btCollisionShape* hoop_left = new btBoxShape(hoopShape);
		collisionShapes.push_back(hoop_left);
		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(leftLocation);

		btScalar mass(0.);
		btVector3 localInertia(0, 0, 0);
		btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, hoop_left, localInertia);
    rbInfo.m_restitution = hoopRestitution;
    rbInfo.m_friction = hoopFriction;
		btRigidBody* body = new btRigidBody(rbInfo);
		dynamicsWorld->addRigidBody(body);
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

unsigned int vb_color;

unsigned int va_tri;
unsigned int vb_tri;
void init_tri() {
  float top_left_x  = -9.0;
  float top_left_z  = -100.0;
  float top_right_x =  9.0;
  float bot_left_z  =  100.0;

  static const GLfloat g_vertex_buffer_data_triange[] = {
    top_left_x,  0.0f, top_left_z,
    top_left_x,  0.0f, bot_left_z,
    top_right_x,  0.0f, top_left_z,
    top_right_x,  0.0f, top_left_z,
    top_left_x,  0.0f, bot_left_z,
    top_right_x,  0.0f, bot_left_z,
  };

  static const GLfloat g_color_buffer_data[] = {
    0.3,  0.3,  0.3,
    0.3,  0.3,  0.3,
    0.3,  0.3,  0.3,
    0.3,  0.3,  0.3,
    0.3,  0.3,  0.3,
    0.3,  0.3,  0.3,
  };

  glGenVertexArrays(1, &va_tri);
  glBindVertexArray(va_tri);
  glGenBuffers(1, &vb_tri);
  glBindBuffer(GL_ARRAY_BUFFER, vb_tri);
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
  std::vector< glm::vec3 > normals; // Won't be used at the moment.
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

unsigned int va_hoop;
unsigned int vb_hoop;
unsigned int va_hoop_tex;
unsigned int vb_hoop_tex;
unsigned int hoop_vertices_size;
void init_hoop(GLuint programID) {
  std::vector<vec3> vertices;
  std::vector<vec2> uvs;
  std::vector<vec3> normals; // Won't be used at the moment.
  bool res = loadOBJ("../playground/hoop.obj", vertices, uvs, normals);
  hoop_vertices_size = vertices.size();

  glGenVertexArrays(1, &va_hoop);
  glBindVertexArray(va_hoop);

  glGenBuffers(1, &vb_hoop);
  glBindBuffer(GL_ARRAY_BUFFER, vb_hoop);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer( 0, // matches the layout in the shader
    3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	va_hoop_tex = loadDDS("../playground/hoop.dds");
  glGenBuffers(1, &vb_hoop_tex);
  glBindBuffer(GL_ARRAY_BUFFER, vb_hoop_tex);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
  glVertexAttribPointer( 1, // matches the layout in the shader
    2, // <- note the 2
    GL_FLOAT, GL_FALSE, 0, (void*)0 );

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

float rotation_ball = 0;
float rotate_ball() {
  rotation_ball = rotation_ball +1 % 360;
  return rotation_ball;
}

float force = 50;
void fireBall(GLFWwindow* window, int button, int action, int mods) {
  // Todo: check if ball is resting
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
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow( 1024, 720, "Playground", NULL, NULL );
  if ( window == NULL ) {
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetMouseButtonCallback(window, fireBall);

  // Initialize GLEW
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwTerminate();
    return -1;
  }

  hoop_location(0, 1, -4);
  btDiscreteDynamicsWorld* dynamicsWorld = initBulletWorld();

  glEnable( GL_DEPTH_TEST ); // Z-Buffer
  glDepthFunc( GL_LESS ); // accept fragments if they are closer to the camera
	glEnable(GL_CULL_FACE); // backface culling

  glClearColor(0.8f, 0.0f, 0.4f, 0.0f);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // shaders
  GLuint programID_Color = LoadShaders( "../common/Simple.vertexshader", "../common/Color.fragmentshader" );
  GLuint programID_Texture = LoadShaders( "../common/Transform.vertexshader", "../common/Texture.fragmentshader" );

  // share MVP data
  mat4 Projection = perspective( radians(45.0f),  4.f/3.f, 0.1f, 100.0f ); // 45° field of view, 4:3, display range: 0.1 unit <-> 100 units
  mat4 View = lookAt(
    vec3(0, 6, 12), // positon of the camera in world space
    vec3(0, 1, 0), // looks at the origin
    vec3(0, 1, 0)  // head is up, -1 to look upside down
  );

    // initialize object buffers
  init_ball(programID_Texture);
  init_hoop(programID_Texture);
  init_tri();

  // ball transforms
  mat4 scale2x = scale(mat4(), vec3(2.5,2.5,2.5));

  // hoop transforms
  mat4 hoopPosition = translate(hoopTranslation, vec3(-17.95f, -6.8f, -2.6f)); // obj has some weird built in coords
  mat4 hoopScale = scale(mat4(), vec3(2.4f, 2.4f, 2.4f));
  mat4 hoopRotation = rotate(mat4(), radians(-90.f), vec3(0, 1, 0));

  // ground/triange MVP
  mat4 translationMatrix_tri = translate(mat4(), vec3(0.0f, 0.0f, 2.0f));

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // keys: https://www.glfw.org/docs/latest/group__keys.html
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
      printf("ENTER -> reset\n");
      resetBallPosition();
    }
    /* if (glfwGetKey(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) { */
    /*   printf("ENTER -> reset\n"); */
    /*   resetBallPosition(); */
    /* } */

    computeMatricesFromInputs(window);
    Projection = getProjectionMatrix();
    View = getViewMatrix();
    mat4 MVP_tri = Projection * View * translationMatrix_tri;

    // increment bullet ball simulation
    dynamicsWorld->stepSimulation(1.f / 60.f, 10);
    btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1];
    btRigidBody* body = btRigidBody::upcast(obj);
    btTransform trans;
			/* if (body && body->getMotionState()) */
			/* 	body->getMotionState()->getWorldTransform(trans); */
    trans = obj->getWorldTransform();

     // texture shaders
    glUseProgram(programID_Texture);
    GLuint MatrixID = glGetUniformLocation(programID_Texture, "MVP");

    { // ball
      float ballX = float(trans.getOrigin().getX());
      float ballY = float(trans.getOrigin().getY());
      float ballZ = float(trans.getOrigin().getZ());
      /* printf("x=%.02f, y=%.02f, z=%.02f, %f\n", 1, ballX, ballY, ballZ ); */
      mat4 matchBallWithSim = translate(mat4(), vec3(ballX, ballY, ballZ));

      if (ballY > 0.6) {
        rotate_ball();
      }

      mat4 ballRotation = rotate(mat4(), radians((float) rotation_ball), vec3(1, 1, 0) );
      mat4 Model = mat4(1) * scale2x * matchBallWithSim * ballRotation;
      mat4 MVP = Projection * View * Model;

      glUniformMatrix4fv( MatrixID, 1, GL_FALSE, &MVP[0][0] );
      glBindVertexArray(va_ball);
      glBindBuffer(GL_ARRAY_BUFFER, vb_ball);
      glBindTexture(GL_TEXTURE_2D, va_ball_tex);
      glBindBuffer(GL_ARRAY_BUFFER, vb_ball_tex);
      glDrawArrays(GL_TRIANGLES, 0, ball_vertices_size);
    }

    { // hoop
      mat4 Model = mat4(1) * hoopPosition * hoopScale * hoopRotation;
      mat4 MVP = Projection * View * Model;

      glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
      glBindVertexArray(va_hoop);
      glBindBuffer(GL_ARRAY_BUFFER, vb_hoop);
      glBindTexture(GL_TEXTURE_2D, va_hoop_tex);
      glBindBuffer(GL_ARRAY_BUFFER, vb_hoop_tex);
      glDrawArrays(GL_TRIANGLES, 0, hoop_vertices_size);
    }

    { // ground
      glUseProgram(programID_Color);
      GLuint MatrixID_Color = glGetUniformLocation(programID_Color, "MVP");

      glUniformMatrix4fv( MatrixID_Color, 1, GL_FALSE, &MVP_tri[0][0] );
      glBindVertexArray(va_tri);
      glBindBuffer(GL_ARRAY_BUFFER, vb_tri);
      glDrawArrays(GL_TRIANGLES, 0, 3*2); // each triange is 3
    }

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
  } while( // Check if the ESC key was pressed or the window was closed
    glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
    glfwWindowShouldClose(window) == 0
  );

  // cleanup
  /* glDeleteBuffers(1, &vb_color); */
  /* glDeleteBuffers(1, &vb_tri); */
  /* glDeleteVertexArrays(1, &va_tri); */
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDeleteProgram(programID_Color);
  glDeleteProgram(programID_Texture);

  glfwTerminate();
  return 0;
}
