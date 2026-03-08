#include <cmath>
#include <filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

// compiler and preprocessor
#include "compiler/compiler/compiler.h"
#include "compiler/preprocessor/preprocessor.h"

#include "camera/camera.h"
#include "render/render.h"

static void error_callback(int error, const char *description);

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods); // openGL callback

/* Camera states */
bool move_right = false;
bool move_up = false;
bool move_down = false;
bool move_left = false;
bool zoom_in = false;
bool zoom_out = false;

int main() {

  const int WIDTH = 1000, HEIGHT = 800;

  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    std::cerr << "GLFW initialization failed\n";
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "Shader run", nullptr, nullptr);

  /*
   * various window checks
   */
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  /*
   * Initialise renderer
   */
  std::filesystem::path v_shader_path =
      std::filesystem::path(SHADERS_DIR) / "vertex_shader.glsl";
  std::filesystem::path f_shader_path =
      std::filesystem::path(SHADERS_DIR) / "fragment_shader.glsl";
  ParticleRenderer renderer;
  if (!renderer.init(v_shader_path, f_shader_path)) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  /*
   * Preparing the viewport: a single 2D rectangle (two triangles)
   */

  float vertices[] = {
      -1.0f, -1.0f, // down left
      1.0f,  -1.0f, // down right
      1.0f,  1.0f,  // up right

      -1.0f, -1.0f, // down left
      1.0f,  1.0f,  // up right
      -1.0f, 1.0f   // up left
  };
  const int _dimension = 2;
  const int _vertex_count =
      sizeof(vertices) / _dimension; // 1 vertex consists of a n-tuple of
                                     // coordinates

  // create memory on the GPU
  unsigned int VBO;
  glGenBuffers(1, &VBO);

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // copy vertices array to those memory
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, _vertex_count * _dimension, vertices,
               GL_STATIC_DRAW);

  // configure how OpenGL should interpret the memory
  glVertexAttribPointer(0, _dimension, GL_FLOAT, GL_FALSE,
                        _dimension * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  /*
   * Initialising a 2D scene
   */
  // coords of the seahorse valley

  // an interesting point
  float _p_x = 0.0;
  float _p_y = 0.0;
  Camera camera(_p_x, _p_y);
  camera.zoom_in(0.8);

  glfwSetWindowUserPointer(window, &camera);

  glfwSetKeyCallback(window, key_callback);
  float _per_frame_camera_stride = 0.5; // per frame camera stride
  float _zoom_in_out_stride = 0.1;
  /*
   * main loop
   */
  while (!glfwWindowShouldClose(window)) {
    float time = (float)glfwGetTime();
    int w, h;
    glfwPollEvents();

    float right_str = move_right ? _per_frame_camera_stride : 0.0;
    float left_str = move_left ? -_per_frame_camera_stride : 0.0;
    float down_str = move_down ? -_per_frame_camera_stride : 0.0;
    float up_str = move_up ? _per_frame_camera_stride : 0.0;
    camera.move(right_str + left_str, up_str + down_str);

    camera.zoom_in(zoom_in ? _zoom_in_out_stride : 0.0);
    camera.zoom_out(zoom_out ? _zoom_in_out_stride : 0.0);

    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f); // background color;
    glClear(GL_COLOR_BUFFER_BIT);

    GLint loc_center_uniform = glGetUniformLocation(shader_program, "uCenter");
    GLint loc_scale_uniform = glGetUniformLocation(shader_program, "uScale");
    GLint loc_aspect_uniform = glGetUniformLocation(shader_program, "uAspect");
    GLint loc_time_uniform = glGetUniformLocation(shader_program, "uTime");

    glUniform2f(loc_center_uniform, camera.get_x(), camera.get_y());
    glUniform1f(loc_scale_uniform, camera.get_zoom() * camera.get_scale());
    glUniform1f(loc_aspect_uniform, (float)w / (float)h);
    glUniform1f(loc_time_uniform, time);

    // rendering
    glDrawArrays(GL_POINTS, 0, _vertex_count);

    glfwSwapBuffers(window);
  }
}

static void error_callback(int error, const char *description) {
  std::cerr << "GLFW error" << error << ": " << description << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {

  if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    move_right = true;
  if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
    move_right = false;

  if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    move_left = true;
  if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
    move_left = false;
  if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    move_up = true;
  if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
    move_up = false;
  if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    move_down = true;
  if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
    move_down = false;
  if (key == GLFW_KEY_O && action == GLFW_PRESS)
    zoom_in = true;
  if (key == GLFW_KEY_O && action == GLFW_RELEASE)
    zoom_in = false;
  if (key == GLFW_KEY_N && action == GLFW_PRESS)
    zoom_out = true;
  if (key == GLFW_KEY_N && action == GLFW_RELEASE)
    zoom_out = false;
}
