#include <cmath>
#include <filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "render/render.h"
#include "simulation/simulation.h"

static void error_callback(int error, const char *description);

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
    std::cerr << "Renderer init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  const size_t kParticles = 1000;
  const size_t kClasses = 1;
  Simulation simulation(kParticles, kClasses);
  if (simulation.size() != kParticles || simulation.class_count() != kClasses) {
    std::cerr << "Simulation init failed: expected " << kParticles
              << " particles and " << kClasses << " classes, got "
              << simulation.size() << " particles and "
              << simulation.class_count() << " classes\n";
    renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  const size_t per_class = kParticles / kClasses;
  const float kPi = 3.14159265358979323846f;
  for (size_t i = 0; i < kParticles; ++i) {
    const int class_id = (i < per_class) ? 0 : 1;
    const size_t class_index = (class_id == 0) ? i : (i - per_class);
    const float t = (per_class > 1) ? static_cast<float>(class_index) /
                                          static_cast<float>(per_class - 1)
                                    : 0.0f;
    const float angle = t * 8.0f * kPi;
    const float radius = 0.35f * std::sqrt(t);
    const float center_x = (class_id == 0) ? -0.5f : 0.5f;
    const float x = center_x + radius * std::cos(angle);
    const float y = radius * std::sin(angle);
    simulation.set_particle(i, x, y, 0.0f, 0.0f, class_id);
  }

  if (simulation.pos_x().size() != simulation.pos_y().size() ||
      simulation.pos_x().size() != simulation.classes().size()) {
    std::cerr << "Simulation data mismatch: pos_x=" << simulation.pos_x().size()
              << ", pos_y=" << simulation.pos_y().size()
              << ", classes=" << simulation.classes().size() << "\n";
    renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  renderer.upload_particles(simulation.pos_x(), simulation.pos_y(),
                            simulation.classes());
  glPointSize(3.0f);
  /*
   * main loop
   */
  while (!glfwWindowShouldClose(window)) {
    int w, h;
    glfwPollEvents();

    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f); // background color;
    glClear(GL_COLOR_BUFFER_BIT);

    renderer.draw(simulation.size());

    glfwSwapBuffers(window);
  }

  renderer.shutdown();
  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}

static void error_callback(int error, const char *description) {
  std::cerr << "GLFW error" << error << ": " << description << std::endl;
}
