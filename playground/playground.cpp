#include <stdio.h>
#include <stdlib.h>

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

unsigned int va_tri;
unsigned int vb_tri;
void init_tri() {
  static const GLfloat g_vertex_buffer_data_triange[] = {
    -3.0f, -3.0f, 0.0f,
    3.0f, -3.0f, 0.0f,
    0.0f,  3.0f, 0.0f,
  };

  glGenVertexArrays(1, &va_tri);
  glBindVertexArray(va_tri);
  glGenBuffers(1, &vb_tri);
  glBindBuffer(GL_ARRAY_BUFFER, vb_tri);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_triange), g_vertex_buffer_data_triange, GL_STATIC_DRAW);
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  glEnableVertexAttribArray(0);
}

unsigned int va_cube;
unsigned int vb_cube;
unsigned int vb_color;
void init_cube() {
  // Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
  // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
  static const GLfloat g_vertex_buffer_data_cube[] = {
    -1.0f,-1.0f,-1.0f, // triangle 1 : begin
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, // triangle 1 : end
    1.0f, 1.0f,-1.0f, // triangle 2 : begin
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f, // triangle 2 : end
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
  };

  // One color for each vertex. They were generated randomly.
  static const GLfloat g_color_buffer_data[] = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f,
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
  };

  static const GLfloat g_uv_buffer_data[] = {
    0.000059f, 1.0f-0.000004f,
    0.000103f, 1.0f-0.336048f,
    0.335973f, 1.0f-0.335903f,
    1.000023f, 1.0f-0.000013f,
    0.667979f, 1.0f-0.335851f,
    0.999958f, 1.0f-0.336064f,
    0.667979f, 1.0f-0.335851f,
    0.336024f, 1.0f-0.671877f,
    0.667969f, 1.0f-0.671889f,
    1.000023f, 1.0f-0.000013f,
    0.668104f, 1.0f-0.000013f,
    0.667979f, 1.0f-0.335851f,
    0.000059f, 1.0f-0.000004f,
    0.335973f, 1.0f-0.335903f,
    0.336098f, 1.0f-0.000071f,
    0.667979f, 1.0f-0.335851f,
    0.335973f, 1.0f-0.335903f,
    0.336024f, 1.0f-0.671877f,
    1.000004f, 1.0f-0.671847f,
    0.999958f, 1.0f-0.336064f,
    0.667979f, 1.0f-0.335851f,
    0.668104f, 1.0f-0.000013f,
    0.335973f, 1.0f-0.335903f,
    0.667979f, 1.0f-0.335851f,
    0.335973f, 1.0f-0.335903f,
    0.668104f, 1.0f-0.000013f,
    0.336098f, 1.0f-0.000071f,
    0.000103f, 1.0f-0.336048f,
    0.000004f, 1.0f-0.671870f,
    0.336024f, 1.0f-0.671877f,
    0.000103f, 1.0f-0.336048f,
    0.336024f, 1.0f-0.671877f,
    0.335973f, 1.0f-0.335903f,
    0.667969f, 1.0f-0.671889f,
    1.000004f, 1.0f-0.671847f,
    0.667979f, 1.0f-0.335851f
  };

  glGenVertexArrays(1, &va_cube);
  glBindVertexArray(va_cube);

  glGenBuffers(1, &vb_cube);
  glBindBuffer(GL_ARRAY_BUFFER, vb_cube);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_cube), g_vertex_buffer_data_cube, GL_STATIC_DRAW);
  glVertexAttribPointer( 0, // matches the layout in the shader
    3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

  bool useTexture = true;
  if (useTexture) {
    glGenBuffers(1, &vb_color);
    glBindBuffer(GL_ARRAY_BUFFER, vb_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer( 1, // matches the layout in the shader
      2, // <- note the 2
      GL_FLOAT, GL_FALSE, 0, (void*)0 );

    const char * imgpath = "../tutorial05_textured_cube/uvtemplate.bmp";
    GLuint Texture = loadBMP_custom("../tutorial05_textured_cube/uvtemplate.bmp");
  } else {
    glGenBuffers(1, &vb_color);
    glBindBuffer(GL_ARRAY_BUFFER, vb_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer( 1, // matches the layout in the shader
      3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  }
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

unsigned int va_ball;
unsigned int vb_ball;
unsigned int vb_ball_tex;
unsigned int ball_vertices_size;
void init_ball(GLuint programID) {
  // Read our .obj file
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

	GLuint Texture = loadDDS("../playground/ball.dds");
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
  glGenBuffers(1, &vb_ball_tex);
  glBindBuffer(GL_ARRAY_BUFFER, vb_ball_tex);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
  glVertexAttribPointer( 1, // matches the layout in the shader
    2, // <- note the 2
    GL_FLOAT, GL_FALSE, 0, (void*)0 );

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

int main( void ) {
  // Initialise GLFW
  glewExperimental = true;
  if( !glfwInit() ) {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    return -1;
  }

  mat4 Einheitsmatrix = mat4(1);
  /* mat4 myMatrix = { */
  /*   1, 0, 0, 0, */
  /*   0, 1, 0, 0, */
  /*   0, 0, 1, 0, */
  /*   10, 0, 0, 1 */
  /* }; */

  /* mat4 translationMatrix = translate(mat4(), vec3(10.0f, 0.0f, 0.0f)); */
  /* vec4 myVector(10.0f, 10.0f, 10.0f, 1.0f); */
  /* vec4 translatedVector = myMatrix * myVector; */

  /* glm::mat4 myScalingMatrix = glm::scale(glm::mat4(1), glm::vec3(2,2,2)); */
  /* glm::vec4 myVector(10.0f, 10.0f, 10.0f, 1.0f); */
  /* glm::vec4 res = myScalingMatrix * myVector; */
  /* print_vec4(res); */

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  GLFWwindow* window = glfwCreateWindow( 1024, 720, "Playground", NULL, NULL );
  if ( window == NULL ) {
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwTerminate();
    return -1;
  }

  glEnable( GL_DEPTH_TEST ); // Z-Buffer
  glDepthFunc( GL_LESS ); // accept fragments if they are closer to the camera
	glEnable(GL_CULL_FACE); // backface culling

  // Create and compile our GLSL program from the shaders
  /* GLuint programID = LoadShaders( "../common/SimpleVertexShader.vertexshader", "../common/SimpleFragmentShader.fragmentshader" ); */
  GLuint programID = LoadShaders( "../common/Transform.vertexshader", "../common/Texture.fragmentshader" );
  /* GLuint programID = LoadShaders( "../common/Texture.vertexshader", "../common/Texture.fragmentshader" ); */
  // get ID for MVP matrix
  GLuint MatrixID = glGetUniformLocation(programID, "MVP");

  glClearColor(0.8f, 0.0f, 0.4f, 0.0f);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // 45° field of view, 4:3, display range: 0.1 unit <-> 100 units
  mat4 Projection = perspective( radians(45.0f), (float) 4/(float) 3, 0.1f, 100.0f );

  mat4 View = lookAt(
    vec3(1, 1, 1), // positon of the camera in world space
    vec3(0, 0, 0), // looks at the origin
    vec3(0, 1, 0)  // head is up, -1 to look upside down
  );

  mat4 translationMatrix = translate(mat4(), vec3(-2.0f, 0.0f, 0.0f));
  mat4 translationMatrix_tri = translate(mat4(), vec3(2.0f, 2.0f, 4.0f));
  mat4 scalehalf = scale(mat4(), vec3(.5,.5,.5));
  mat4 scale2x = scale(mat4(), vec3(2.5,2.5,2.5));
  mat4 rotationMatrix = rotate(mat4(), radians(15.0f), vec3(1, 0, 0) );
  /* mat4 Model = rotationMatrix * translationMatrix * scalingMatrix; */
  mat4 Model = mat4(1) * scale2x;
  mat4 Model_tri = scalehalf * translationMatrix_tri;

  mat4 MVP_cube = Projection * View * Model;
  mat4 MVP_tri = Projection * View * Model_tri;

  /* init_tri(); */
  /* init_cube(); */
  init_ball(programID);

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the shader
    glUseProgram(programID);

    // send our transformations to the currently bound shader, in the MVP uniform
		computeMatricesFromInputs(window);
    mat4 proj = getProjectionMatrix();
    mat4 view = getViewMatrix();
    mat4 MVP = proj * view * Model;
    /* glUniformMatrix4fv( MatrixID, 1, GL_FALSE, &MVP_cube[0][0] ); */
    glUniformMatrix4fv( MatrixID, 1, GL_FALSE, &MVP[0][0] );
    /* glBindBuffer(GL_ARRAY_BUFFER, vb_cube); */
    /* glBindVertexArray(va_cube); */
    /* glDrawArrays(GL_TRIANGLES, 0, 12*3); // each triange is 3 */

    glBindBuffer(GL_ARRAY_BUFFER, vb_ball);
    glBindVertexArray(va_ball);
    glDrawArrays(GL_TRIANGLES, 0, ball_vertices_size); // each triange is 3

    // 2nd draw
    /* glUniformMatrix4fv( MatrixID, 1, GL_FALSE, &MVP_tri[0][0] ); */
    /* glBindBuffer(GL_ARRAY_BUFFER, vb_tri); */
    /* glBindVertexArray(va_tri); */
    /* glDrawArrays(GL_TRIANGLES, 0, 3); // each triange is 3 */

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
  } while( // Check if the ESC key was pressed or the window was closed
    glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
    glfwWindowShouldClose(window) == 0
  );

  // cleanup
  /* glDeleteBuffers(1, &vb_cube); */
  /* glDeleteBuffers(1, &vb_color); */
  /* glDeleteBuffers(1, &vb_tri); */
  /* glDeleteVertexArrays(1, &va_tri); */
  /* glDeleteVertexArrays(1, &va_cube); */
  /* glDisableVertexAttribArray(0); */
  /* glDisableVertexAttribArray(1); */
  /* glDeleteProgram(programID); */

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}
