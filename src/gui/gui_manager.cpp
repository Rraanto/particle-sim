/*
 * Implementation of GUIManager using Dear ImGui
 */

#include "gui_manager.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "simulation.h"
#include "params.h"
#include "render.h"

#include <GLFW/glfw3.h>
#include <cstdio>
#include <algorithm>

namespace {

constexpr size_t kFppsOptions[] = {1u, 2u, 4u, 8u, 12u};
const char *kFppsLabels[] = {"1", "2", "4", "8", "12"};

int fpps_to_index(size_t fpps) {
  for (size_t i = 0; i < std::size(kFppsOptions); ++i) {
    if (kFppsOptions[i] == fpps) {
      return static_cast<int>(i);
    }
  }
  return 0;
}

size_t index_to_fpps(int index) {
  if (index < 0 || static_cast<size_t>(index) >= std::size(kFppsOptions)) {
    return kFppsOptions[0];
  }
  return kFppsOptions[static_cast<size_t>(index)];
}

} // namespace

bool GUIManager::init(GLFWwindow *window) {
  if (_initialized) {
    return true;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  if (!ImGui_ImplGlfw_InitForOpenGL(window, false)) {
    ImGui::DestroyContext();
    return false;
  }
  if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return false;
  }

  ImGui::StyleColorsDark();

  _initialized = true;
  return true;
}

void GUIManager::shutdown() {
  if (!_initialized) {
    return;
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  _initialized = false;
}

void GUIManager::render(Simulation &sim, ParticleRenderer &renderer) {
  (void)renderer;
  if (!_initialized) {
    return;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  render_global_params_window(sim);
  render_setup_window();
  render_class_editor_window(sim, renderer);
  render_diagnostic_overlay(sim);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool GUIManager::want_capture_mouse() const {
  if (!_initialized) {
    return false;
  }
  return ImGui::GetIO().WantCaptureMouse;
}

bool GUIManager::want_capture_keyboard() const {
  if (!_initialized) {
    return false;
  }
  return ImGui::GetIO().WantCaptureKeyboard;
}

void GUIManager::set_fpps(size_t fpps) {
  _active_config.fpps = index_to_fpps(fpps_to_index(fpps));
  _pending_config.fpps = _active_config.fpps;
}

void GUIManager::set_step_time(double step_time_ms) {
  _last_step_time_ms = step_time_ms;
}

void GUIManager::sync_config(const AppConfig &config) {
  _active_config = config;
  _pending_config = config;
}

void GUIManager::render_global_params_window(Simulation &sim) {
  ImGui::Begin("Global Parameters");

  if (ImGui::SliderFloat("Attraction", &_active_config.attraction_strength, -1.0f, 1.0f)) {
    sim.set_attraction_strength(_active_config.attraction_strength);
  }

  if (ImGui::SliderFloat("Damping", &_active_config.damping, 0.5f, 1.5f)) {
    sim.set_damping(_active_config.damping);
  }

  if (ImGui::SliderFloat("Neighbor Radius", &_active_config.interaction_radius, 0.1f, 1.0f)) {
    sim.set_interaction_radius(_active_config.interaction_radius);
  }

  if (ImGui::SliderFloat("Max Speed", &_active_config.max_speed, 0.1f, 10.0f)) {
    sim.set_max_speed(_active_config.max_speed);
  }

  int fpps_index = fpps_to_index(_active_config.fpps);
  if (ImGui::Combo("FPPS", &fpps_index, kFppsLabels,
                   static_cast<int>(std::size(kFppsLabels)))) {
    _active_config.fpps = index_to_fpps(fpps_index);
  }
  ImGui::Text("Physics every %zu frame(s)", _active_config.fpps);

  ImGui::Separator();
  ImGui::Checkbox("Pause", &_active_config.paused);
  ImGui::SameLine();
  if (ImGui::Checkbox("Wrap Bounds", &_active_config.wrap_bounds)) {
    // Note: Simulation class might need a setter for wrap_bounds if we want live update.
    // For now, it's structural in AppConfig but let's assume it can be live.
    // Actually SimulationParams has it. Let's add a setter to Simulation if needed.
  }

  if (ImGui::Button("Reset Particles")) {
    sim.reset_particles();
  }

  ImGui::End();
}

void GUIManager::render_setup_window() {
  ImGui::Begin("Simulation Setup");
  ImGui::Text("Structural parameters (Require Restart)");

  int class_count = static_cast<int>(_pending_config.class_count);
  if (ImGui::InputInt("Classes", &class_count)) {
    _pending_config.class_count = static_cast<size_t>(std::max(1, class_count));
    _pending_config.synchronize_dimensions();
  }

  int per_class = static_cast<int>(_pending_config.particles_per_class);
  if (ImGui::InputInt("Particles/Class", &per_class)) {
    _pending_config.particles_per_class = static_cast<size_t>(std::max(1, per_class));
  }

  ImGui::SliderFloat("Grid Size", &_pending_config.grid_cell_size, 0.01f, 1.0f);
  ImGui::InputFloat("Time Step", &_pending_config.time_step, 0.001f, 0.1f, "%.4f");

  ImGui::Separator();

  bool changed = (_pending_config.class_count != _active_config.class_count) ||
                 (_pending_config.particles_per_class != _active_config.particles_per_class) ||
                 (_pending_config.grid_cell_size != _active_config.grid_cell_size) ||
                 (_pending_config.time_step != _active_config.time_step);

  if (changed) {
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Changes pending...");
    if (ImGui::Button("Apply & Restart Simulation")) {
      _restart_requested = true;
    }
  } else {
    ImGui::Text("Configuration is up to date.");
    ImGui::BeginDisabled();
    ImGui::Button("Apply & Restart Simulation");
    ImGui::EndDisabled();
  }

  ImGui::End();
}

void GUIManager::render_class_editor_window(Simulation &sim,
                                            ParticleRenderer &renderer) {
  (void)renderer;
  ImGui::Begin("Class Editor");

  const size_t class_count = _active_config.class_count;
  if (class_count == 0) {
    ImGui::Text("No classes available");
    ImGui::End();
    return;
  }

  if (_selected_class < 0) {
    _selected_class = 0;
  }
  if (static_cast<size_t>(_selected_class) >= class_count) {
    _selected_class = static_cast<int>(class_count - 1);
  }

  char selected_label[32];
  std::snprintf(selected_label, sizeof(selected_label), "Class %d",
                _selected_class);
  if (ImGui::BeginCombo("Select Class", selected_label)) {
    for (size_t class_index = 0; class_index < class_count; ++class_index) {
      char class_label[32];
      std::snprintf(class_label, sizeof(class_label), "Class %zu", class_index);
      const bool is_selected =
          static_cast<size_t>(_selected_class) == class_index;
      if (ImGui::Selectable(class_label, is_selected)) {
        _selected_class = static_cast<int>(class_index);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  ImGui::Separator();

  const size_t selected_class = static_cast<size_t>(_selected_class);
  if (selected_class < _active_config.classes.size()) {
    auto &class_param = _active_config.classes[selected_class];
    float color[3] = {class_param.r, class_param.g, class_param.b};
    if (ImGui::ColorEdit3("Color", color)) {
      class_param.r = color[0];
      class_param.g = color[1];
      class_param.b = color[2];
      sim.set_class_color(selected_class, color[0], color[1], color[2]);
    }

    if (ImGui::SliderFloat("Mass", &class_param.mass, 0.1f, 10.0f)) {
      sim.set_class_mass(selected_class, class_param.mass);
    }
  }

  ImGui::Separator();
  ImGui::Text("Attraction Matrix");

  const float input_width = 72.0f;
  for (size_t target_class = 0; target_class < class_count; ++target_class) {
    ImGui::PushID(static_cast<int>(target_class));
    float &value = _active_config.attraction_matrix[selected_class * class_count + target_class];
    ImGui::SetNextItemWidth(input_width);

    char label[24];
    std::snprintf(label, sizeof(label), "A[%d][%zu]", _selected_class,
                  target_class);
    if (ImGui::InputFloat(label, &value, 0.0f, 0.0f, "%.3f")) {
      sim.set_attraction(selected_class, target_class, value);
    }

    ImGui::PopID();
    if (target_class + 1 < class_count && (target_class + 1) % 4 != 0) {
      ImGui::SameLine();
    }
  }

  ImGui::End();
}

void GUIManager::render_diagnostic_overlay(const Simulation &sim) {
  ImGuiIO &io = ImGui::GetIO();
  const ImVec2 window_pos(io.DisplaySize.x - 10.0f, 10.0f);
  const ImVec2 window_pos_pivot(1.0f, 0.0f);

  ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
  ImGui::SetNextWindowBgAlpha(0.35f);

  const ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration |
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

  ImGui::Begin("Diagnostic Dashboard", nullptr, window_flags);
  ImGui::Text("FPS: %.1f", io.Framerate);
  ImGui::Text("Particles: %zu", sim.size());
  ImGui::Text("Step Time: %.3f ms", _last_step_time_ms);
  ImGui::End();
}
