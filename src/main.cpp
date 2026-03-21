#include <filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <string>
#include <array>

#include "render/render.h"
#include "simulation/simulation.h"
#include "camera.h"

static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods);
static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                  int mods);
static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

enum class Action {
  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  ZoomIn,
  ZoomOut,
  Count
};

struct InputState {
  std::array<bool, static_cast<size_t>(Action::Count)> active{};
  bool dragging = false;
  double last_x = 0.0;
  double last_y = 0.0;
  double cursor_x = 0.0;
  double cursor_y = 0.0;
  bool cursor_valid = false;
  Camera *camera = nullptr;
};

static bool map_key_to_action(int key, Action &out_action);
static void apply_input(const InputState &input, Camera &camera);
static void apply_mouse_drag(InputState &input, Camera &camera,
                             GLFWwindow *window);

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

  Camera camera(0.0f, 0.0f);
  InputState input_state;
  input_state.camera = &camera;
  glfwSetWindowUserPointer(window, &input_state);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetScrollCallback(window, scroll_callback);

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

  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> dist_x(-1.0f, 1.0f);
  std::uniform_real_distribution<float> dist_y(-1.0f, 1.0f);
  for (size_t i = 0; i < kParticles; ++i) {
    const float x = dist_x(rng);
    const float y = dist_y(rng);
    simulation.set_particle(i, x, y, 0.5f, 0.5f, 0);
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
  double last_time = glfwGetTime();
  double accumulator = 0.0;
  const double fixed_dt = 1.0 / 60.0;
  while (!glfwWindowShouldClose(window)) {
    int w, h;
    glfwPollEvents();

    // Input step (separate from simulation/rendering).
    apply_input(input_state, camera);
    apply_mouse_drag(input_state, camera, window);

    const double now = glfwGetTime();
    double frame_dt = now - last_time;
    last_time = now;
    if (frame_dt < 0.0) {
      frame_dt = 0.0;
    }
    // Prevent huge catch-up steps if the event loop stalls.
    if (frame_dt > 0.25) {
      frame_dt = 0.25;
    }
    accumulator += frame_dt;
    while (accumulator >= fixed_dt) {
      simulation.step(static_cast<float>(fixed_dt));
      accumulator -= fixed_dt;
    }
    renderer.upload_particles(simulation.pos_x(), simulation.pos_y(),
                              simulation.classes());

    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f); // background color;
    glClear(GL_COLOR_BUFFER_BIT);

    const float aspect =
        (h > 0) ? static_cast<float>(w) / static_cast<float>(h) : 1.0f;
    const float time_sec = static_cast<float>(glfwGetTime());
    renderer.draw(simulation.size(), camera, aspect, time_sec);

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

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  (void)scancode;
  (void)mods;
  if (action != GLFW_PRESS && action != GLFW_RELEASE) {
    return;
  }

  InputState *input =
      static_cast<InputState *>(glfwGetWindowUserPointer(window));
  if (input == nullptr) {
    return;
  }

  Action mapped_action;
  if (!map_key_to_action(key, mapped_action)) {
    return;
  }

  const bool pressed = (action == GLFW_PRESS);
  input->active[static_cast<size_t>(mapped_action)] = pressed;
}

static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                  int mods) {
  (void)mods;
  if (button != GLFW_MOUSE_BUTTON_LEFT) {
    return;
  }

  InputState *input =
      static_cast<InputState *>(glfwGetWindowUserPointer(window));
  if (input == nullptr) {
    return;
  }

  if (action == GLFW_PRESS) {
    input->dragging = true;
    glfwGetCursorPos(window, &input->last_x, &input->last_y);
    input->cursor_x = input->last_x;
    input->cursor_y = input->last_y;
    input->cursor_valid = true;
  } else if (action == GLFW_RELEASE) {
    input->dragging = false;
  }
}

static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
  InputState *input =
      static_cast<InputState *>(glfwGetWindowUserPointer(window));
  if (input == nullptr) {
    return;
  }
  input->cursor_x = xpos;
  input->cursor_y = ypos;
  input->cursor_valid = true;
}

static void apply_mouse_drag(InputState &input, Camera &camera,
                             GLFWwindow *window) {
  if (!input.dragging || !input.cursor_valid) {
    return;
  }

  const double dx = input.cursor_x - input.last_x;
  const double dy = input.cursor_y - input.last_y;
  input.last_x = input.cursor_x;
  input.last_y = input.cursor_y;

  if (dx == 0.0 && dy == 0.0) {
    return;
  }

  int w = 0, h = 0;
  glfwGetFramebufferSize(window, &w, &h);
  if (w <= 0 || h <= 0) {
    return;
  }

  const float ndc_dx = static_cast<float>(2.0 * dx / w);
  const float ndc_dy = static_cast<float>(-2.0 * dy / h);
  const float aspect = static_cast<float>(w) / static_cast<float>(h);
  const float scale = camera.get_scale();
  const float zoom = camera.get_zoom();

  const float world_dx = ndc_dx * aspect * scale * zoom;
  const float world_dy = ndc_dy * scale * zoom;

  // Dragging moves the world with the cursor (camera moves opposite).
  camera.move(-world_dx, -world_dy);
}

static void scroll_callback(GLFWwindow *window, double xoffset,
                            double yoffset) {
  (void)xoffset;
  if (yoffset == 0.0) {
    return;
  }

  InputState *input =
      static_cast<InputState *>(glfwGetWindowUserPointer(window));
  if (input == nullptr || input->camera == nullptr) {
    return;
  }

  int w = 0, h = 0;
  glfwGetFramebufferSize(window, &w, &h);
  if (w <= 0 || h <= 0) {
    return;
  }

  double xpos = 0.0, ypos = 0.0;
  glfwGetCursorPos(window, &xpos, &ypos);

  const float ndc_x = static_cast<float>(2.0 * xpos / w - 1.0);
  const float ndc_y = static_cast<float>(1.0 - 2.0 * ypos / h);
  const float aspect = static_cast<float>(w) / static_cast<float>(h);
  const float scale = input->camera->get_scale();

  const float zoom_before = input->camera->get_zoom();
  const float world_x =
      input->camera->get_x() + ndc_x * aspect * scale * zoom_before;
  const float world_y = input->camera->get_y() + ndc_y * scale * zoom_before;

  const float zoom_stride = 0.1f * static_cast<float>(std::abs(yoffset));
  const float clamped_stride = std::min(0.5f, zoom_stride);
  if (yoffset > 0.0) {
    input->camera->zoom_in(clamped_stride);
  } else {
    input->camera->zoom_out(clamped_stride);
  }

  const float zoom_after = input->camera->get_zoom();
  const float new_center_x = world_x - ndc_x * aspect * scale * zoom_after;
  const float new_center_y = world_y - ndc_y * scale * zoom_after;
  input->camera->move(new_center_x - input->camera->get_x(),
                      new_center_y - input->camera->get_y());
}

static bool map_key_to_action(int key, Action &out_action) {
  switch (key) {
  case GLFW_KEY_W:
  case GLFW_KEY_UP:
    out_action = Action::MoveUp;
    return true;
  case GLFW_KEY_S:
  case GLFW_KEY_DOWN:
    out_action = Action::MoveDown;
    return true;
  case GLFW_KEY_A:
  case GLFW_KEY_LEFT:
    out_action = Action::MoveLeft;
    return true;
  case GLFW_KEY_D:
  case GLFW_KEY_RIGHT:
    out_action = Action::MoveRight;
    return true;
  case GLFW_KEY_Q:
  case GLFW_KEY_EQUAL:
  case GLFW_KEY_KP_ADD:
    out_action = Action::ZoomIn;
    return true;
  case GLFW_KEY_E:
  case GLFW_KEY_MINUS:
  case GLFW_KEY_KP_SUBTRACT:
    out_action = Action::ZoomOut;
    return true;
  default:
    return false;
  }
}

static void apply_input(const InputState &input, Camera &camera) {
  const float move_stride = 0.1f;
  const float zoom_stride = 0.1f;
  const float move_scale = camera.get_zoom();

  if (input.active[static_cast<size_t>(Action::MoveUp)]) {
    camera.move(0.0f, move_stride * move_scale);
  }
  if (input.active[static_cast<size_t>(Action::MoveDown)]) {
    camera.move(0.0f, -move_stride * move_scale);
  }
  if (input.active[static_cast<size_t>(Action::MoveLeft)]) {
    camera.move(-move_stride * move_scale, 0.0f);
  }
  if (input.active[static_cast<size_t>(Action::MoveRight)]) {
    camera.move(move_stride * move_scale, 0.0f);
  }
  if (input.active[static_cast<size_t>(Action::ZoomIn)]) {
    camera.zoom_in(zoom_stride);
  }
  if (input.active[static_cast<size_t>(Action::ZoomOut)]) {
    camera.zoom_out(zoom_stride);
  }
}
