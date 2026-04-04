#include "app.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

#include "camera.h"
#include "gui_manager.h"
#include "render.h"
#include "simulation.h"

namespace {

constexpr int kWindowWidth = 1000;
constexpr int kWindowHeight = 800;
constexpr double kFixedDt = 1.0 / 60.0;

} // namespace

App::App() = default;
App::~App() = default;

bool App::init(const SimConfig &config) {
  _camera = std::make_unique<Camera>(0.0f, 0.0f);
  _renderer = std::make_unique<ParticleRenderer>();
  _gui_manager = std::make_unique<GUIManager>();

  if (!init_window()) {
    shutdown();
    return false;
  }

  if (!init_renderer()) {
    shutdown();
    return false;
  }

  if (!_gui_manager->init(_window)) {
    std::cerr << "GUI Manager init failed\n";
    shutdown();
    return false;
  }
  _gui_manager->set_fpps(config.fpps);

  if (!init_simulation(config)) {
    shutdown();
    return false;
  }

  register_callbacks();
  return true;
}

void App::run() {
  if (_window == nullptr || _simulation == nullptr) {
    return;
  }

  double last_time = glfwGetTime();
  double accumulator = 0.0;
  size_t frame_index = 0;

  while (!glfwWindowShouldClose(_window)) {
    // Event phase: pump GLFW and update the internal input snapshot.
    process_events();

    const double now = glfwGetTime();
    double frame_dt = now - last_time;
    last_time = now;
    if (frame_dt < 0.0) {
      frame_dt = 0.0;
    }
    if (frame_dt > 0.25) {
      frame_dt = 0.25;
    }

    // Update phase: run the fixed-timestep simulation loop.
    update(frame_dt, accumulator, kFixedDt, frame_index);

    // Render phase: upload state, draw particles, draw GUI, then present.
    render();
  }
}

void App::shutdown() {
  if (_window != nullptr) {
    if (_gui_manager != nullptr) {
      _gui_manager->shutdown();
    }
    if (_renderer != nullptr) {
      _renderer->shutdown();
    }
    glfwDestroyWindow(_window);
    _window = nullptr;
  }

  _simulation.reset();
  _camera.reset();
  _gui_manager.reset();
  _renderer.reset();

  if (_glfw_initialized) {
    glfwTerminate();
    _glfw_initialized = false;
  }
}

bool App::init_window() {
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit()) {
    std::cerr << "GLFW initialization failed\n";
    return false;
  }
  _glfw_initialized = true;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  _window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Shader run", nullptr,
                             nullptr);
  if (_window == nullptr) {
    std::cerr << "Failed to create GLFW window\n";
    return false;
  }

  glfwMakeContextCurrent(_window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return false;
  }

  return true;
}

bool App::init_renderer() {
  const std::filesystem::path vertex_shader_path =
      std::filesystem::path(SHADERS_DIR) / "vertex_shader.glsl";
  const std::filesystem::path fragment_shader_path =
      std::filesystem::path(SHADERS_DIR) / "fragment_shader.glsl";

  if (!_renderer->init(vertex_shader_path, fragment_shader_path)) {
    std::cerr << "Renderer init failed\n";
    return false;
  }

  return true;
}

bool App::init_simulation(const SimConfig &config) {
  const size_t class_count = config.class_count;
  const size_t particles_per_class = config.particles_per_class;
  const size_t particle_count = class_count * particles_per_class;

  if (particles_per_class != 0 &&
      particle_count / class_count != particles_per_class) {
    std::cerr << "Particle count overflow (classes * per-class too large)\n";
    return false;
  }

  SimulationParams simulation_params;
  simulation_params.particle_count = particle_count;
  simulation_params.class_count = class_count;
  simulation_params.interaction_radius = config.interaction_radius;
  simulation_params.damping = config.damping;
  simulation_params.grid_cell_size = config.grid_cell_size;
  simulation_params.wrap_bounds = false;

  std::vector<ClassParams> class_params(class_count);
  ForceParams force_params;
  force_params.class_count = class_count;
  if (config.attraction_enabled) {
    force_params.strength = config.attraction_strength;
    force_params.attraction.assign(class_count * class_count, 1.0f);
  }

  _simulation = std::make_unique<Simulation>(simulation_params, class_params,
                                             force_params);
  if (_simulation->size() != particle_count ||
      _simulation->class_count() != class_count) {
    std::cerr << "Simulation init failed: expected " << particle_count
              << " particles and " << class_count << " classes, got "
              << _simulation->size() << " particles and "
              << _simulation->class_count() << " classes\n";
    return false;
  }

  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> dist_x(-1.0f, 1.0f);
  std::uniform_real_distribution<float> dist_y(-1.0f, 1.0f);
  size_t particle_index = 0;
  for (size_t class_id = 0; class_id < class_count; ++class_id) {
    for (size_t particle = 0; particle < particles_per_class; ++particle) {
      const float x = dist_x(rng);
      const float y = dist_y(rng);
      _simulation->set_particle(particle_index, x, y, 0.0f, 0.0f,
                                static_cast<int>(class_id));
      ++particle_index;
    }
  }

  if (_simulation->pos_x().size() != _simulation->pos_y().size() ||
      _simulation->pos_x().size() != _simulation->classes().size()) {
    std::cerr << "Simulation data mismatch: pos_x=" << _simulation->pos_x().size()
              << ", pos_y=" << _simulation->pos_y().size()
              << ", classes=" << _simulation->classes().size() << "\n";
    return false;
  }

  return true;
}

void App::register_callbacks() {
  glfwSetWindowUserPointer(_window, this);
  glfwSetKeyCallback(_window, key_callback);
  glfwSetMouseButtonCallback(_window, mouse_button_callback);
  glfwSetCursorPosCallback(_window, cursor_pos_callback);
  glfwSetScrollCallback(_window, scroll_callback);
  glfwSetCharCallback(_window, char_callback);
}

void App::process_events() {
  glfwPollEvents();

  if (!_gui_manager->want_capture_keyboard()) {
    apply_input();
  }
  if (!_gui_manager->want_capture_mouse()) {
    apply_mouse_drag();
  }
}

void App::update(double frame_dt, double &accumulator, double fixed_dt,
                 size_t &frame_index) {
  ++frame_index;
  const size_t current_fpps = _gui_manager->fpps();
  if (frame_index % current_fpps != 0) {
    return;
  }

  accumulator += frame_dt;
  while (accumulator >= fixed_dt) {
    if (!_gui_manager->is_paused()) {
      const auto step_start = std::chrono::steady_clock::now();
      _simulation->step(static_cast<float>(fixed_dt));
      const auto step_end = std::chrono::steady_clock::now();
      _gui_manager->set_step_time(
          std::chrono::duration<double, std::milli>(step_end - step_start)
              .count());
    }
    accumulator -= fixed_dt;
  }
}

void App::render() {
  _renderer->upload_particles(_simulation->pos_x(), _simulation->pos_y(),
                              _simulation->classes(),
                              _simulation->class_params());

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(_window, &width, &height);
  glViewport(0, 0, width, height);
  glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
  glClear(GL_COLOR_BUFFER_BIT);

  const float aspect =
      (height > 0) ? static_cast<float>(width) / static_cast<float>(height)
                   : 1.0f;
  const float time_sec = static_cast<float>(glfwGetTime());
  _renderer->draw(_simulation->size(), *_camera, aspect, time_sec);
  _gui_manager->render(*_simulation, *_renderer);

  glfwSwapBuffers(_window);
}

void App::apply_input() {
  const float move_stride = 0.1f;
  const float zoom_stride = 0.1f;
  const float move_scale = _camera->get_zoom();

  if (_input_manager.active[static_cast<size_t>(Action::MoveUp)]) {
    _camera->move(0.0f, move_stride * move_scale);
  }
  if (_input_manager.active[static_cast<size_t>(Action::MoveDown)]) {
    _camera->move(0.0f, -move_stride * move_scale);
  }
  if (_input_manager.active[static_cast<size_t>(Action::MoveLeft)]) {
    _camera->move(-move_stride * move_scale, 0.0f);
  }
  if (_input_manager.active[static_cast<size_t>(Action::MoveRight)]) {
    _camera->move(move_stride * move_scale, 0.0f);
  }
  if (_input_manager.active[static_cast<size_t>(Action::ZoomIn)]) {
    _camera->zoom_in(zoom_stride);
  }
  if (_input_manager.active[static_cast<size_t>(Action::ZoomOut)]) {
    _camera->zoom_out(zoom_stride);
  }
}

void App::apply_mouse_drag() {
  if (!_input_manager.dragging || !_input_manager.cursor_valid) {
    return;
  }

  const double dx = _input_manager.cursor_x - _input_manager.last_x;
  const double dy = _input_manager.cursor_y - _input_manager.last_y;
  _input_manager.last_x = _input_manager.cursor_x;
  _input_manager.last_y = _input_manager.cursor_y;

  if (dx == 0.0 && dy == 0.0) {
    return;
  }

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(_window, &width, &height);
  if (width <= 0 || height <= 0) {
    return;
  }

  const float ndc_dx = static_cast<float>(2.0 * dx / width);
  const float ndc_dy = static_cast<float>(-2.0 * dy / height);
  const float aspect = static_cast<float>(width) / static_cast<float>(height);
  const float scale = _camera->get_scale();
  const float zoom = _camera->get_zoom();

  const float world_dx = ndc_dx * aspect * scale * zoom;
  const float world_dy = ndc_dy * scale * zoom;
  _camera->move(-world_dx, -world_dy);
}

App *App::get_app(GLFWwindow *window) {
  if (window == nullptr) {
    return nullptr;
  }

  return static_cast<App *>(glfwGetWindowUserPointer(window));
}

void App::glfw_error_callback(int error, const char *description) {
  std::cerr << "GLFW error" << error << ": " << description << std::endl;
}

void App::key_callback(GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
  App *app = get_app(window);
  if (app != nullptr) {
    app->on_key(key, scancode, action, mods);
  }
}

void App::mouse_button_callback(GLFWwindow *window, int button, int action,
                                int mods) {
  App *app = get_app(window);
  if (app != nullptr) {
    app->on_mouse_button(button, action, mods);
  }
}

void App::cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
  App *app = get_app(window);
  if (app != nullptr) {
    app->on_cursor_pos(xpos, ypos);
  }
}

void App::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  App *app = get_app(window);
  if (app != nullptr) {
    app->on_scroll(xoffset, yoffset);
  }
}

void App::char_callback(GLFWwindow *window, unsigned int codepoint) {
  App *app = get_app(window);
  if (app != nullptr) {
    app->on_char(codepoint);
  }
}

void App::on_key(int key, int scancode, int action, int mods) {
  ImGui_ImplGlfw_KeyCallback(_window, key, scancode, action, mods);

  if (action != GLFW_PRESS && action != GLFW_RELEASE) {
    return;
  }

  Action mapped_action;
  if (!map_key_to_action(key, mapped_action)) {
    return;
  }

  _input_manager.active[static_cast<size_t>(mapped_action)] =
      (action == GLFW_PRESS);
}

void App::on_mouse_button(int button, int action, int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(_window, button, action, mods);

  if (button != GLFW_MOUSE_BUTTON_LEFT) {
    return;
  }

  if (_gui_manager->want_capture_mouse()) {
    if (action == GLFW_RELEASE) {
      _input_manager.dragging = false;
    }
    return;
  }

  if (action == GLFW_PRESS) {
    _input_manager.dragging = true;
    glfwGetCursorPos(_window, &_input_manager.last_x, &_input_manager.last_y);
    _input_manager.cursor_x = _input_manager.last_x;
    _input_manager.cursor_y = _input_manager.last_y;
    _input_manager.cursor_valid = true;
  } else if (action == GLFW_RELEASE) {
    _input_manager.dragging = false;
  }
}

void App::on_cursor_pos(double xpos, double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(_window, xpos, ypos);
  _input_manager.cursor_x = xpos;
  _input_manager.cursor_y = ypos;
  _input_manager.cursor_valid = true;
}

void App::on_scroll(double xoffset, double yoffset) {
  ImGui_ImplGlfw_ScrollCallback(_window, xoffset, yoffset);

  if (yoffset == 0.0 || _gui_manager->want_capture_mouse()) {
    return;
  }

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(_window, &width, &height);
  if (width <= 0 || height <= 0) {
    return;
  }

  double xpos = 0.0;
  double ypos = 0.0;
  glfwGetCursorPos(_window, &xpos, &ypos);

  const float ndc_x = static_cast<float>(2.0 * xpos / width - 1.0);
  const float ndc_y = static_cast<float>(1.0 - 2.0 * ypos / height);
  const float aspect = static_cast<float>(width) / static_cast<float>(height);
  const float scale = _camera->get_scale();

  const float zoom_before = _camera->get_zoom();
  const float world_x =
      _camera->get_x() + ndc_x * aspect * scale * zoom_before;
  const float world_y = _camera->get_y() + ndc_y * scale * zoom_before;

  const float zoom_stride = 0.1f * static_cast<float>(std::abs(yoffset));
  const float clamped_stride = std::min(0.5f, zoom_stride);
  if (yoffset > 0.0) {
    _camera->zoom_in(clamped_stride);
  } else {
    _camera->zoom_out(clamped_stride);
  }

  const float zoom_after = _camera->get_zoom();
  const float new_center_x = world_x - ndc_x * aspect * scale * zoom_after;
  const float new_center_y = world_y - ndc_y * scale * zoom_after;
  _camera->move(new_center_x - _camera->get_x(),
                new_center_y - _camera->get_y());
}

void App::on_char(unsigned int codepoint) {
  ImGui_ImplGlfw_CharCallback(_window, codepoint);
}

bool App::map_key_to_action(int key, Action &out_action) const {
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
