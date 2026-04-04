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

void GUIManager::set_fpps(size_t fpps) { _fpps = index_to_fpps(fpps_to_index(fpps)); }

void GUIManager::set_step_time(double step_time_ms) {
  _last_step_time_ms = step_time_ms;
}

void GUIManager::render_global_params_window(Simulation &sim) {
  ImGui::Begin("Global Parameters");

  const SimulationParams &params = sim.params();
  const ForceParams &force_params = sim.force_params();

  float strength = force_params.strength;
  if (ImGui::SliderFloat("Attraction", &strength, -1.0f, 1.0f)) {
    sim.set_attraction_strength(strength);
  }

  float damping = params.damping;
  if (ImGui::SliderFloat("Damping", &damping, 0.5f, 1.5f)) {
    sim.set_damping(damping);
  }

  float interaction_radius = params.interaction_radius;
  if (ImGui::SliderFloat("Neighbor Radius", &interaction_radius, 0.1f, 1.0f)) {
    sim.set_interaction_radius(interaction_radius);
  }

  // The GUI is intentionally restricted to the same discrete values as the CLI.
  int fpps_index = fpps_to_index(_fpps);
  if (ImGui::Combo("FPPS", &fpps_index, kFppsLabels,
                   static_cast<int>(std::size(kFppsLabels)))) {
    _fpps = index_to_fpps(fpps_index);
  }
  ImGui::Text("Physics every %zu frame(s)", _fpps);

  ImGui::Separator();
  ImGui::Checkbox("Pause", &_paused);

  if (ImGui::Button("Reset")) {
    sim.reset_particles();
  }

  ImGui::End();
}

void GUIManager::render_class_editor_window(Simulation &sim,
                                            ParticleRenderer &renderer) {
  (void)renderer;
  ImGui::Begin("Class Editor");

  const size_t class_count = sim.class_count();
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

  const std::vector<ClassParams> &class_params = sim.class_params();
  const size_t selected_class = static_cast<size_t>(_selected_class);
  if (selected_class < class_params.size()) {
    const ClassParams &class_param = class_params[selected_class];
    float color[3] = {class_param.r, class_param.g, class_param.b};
    if (ImGui::ColorEdit3("Color", color)) {
      sim.set_class_color(selected_class, color[0], color[1], color[2]);
    }

    float mass = class_param.mass;
    if (ImGui::SliderFloat("Mass", &mass, 0.1f, 10.0f)) {
      sim.set_class_mass(selected_class, mass);
    }
  }

  ImGui::Separator();
  ImGui::Text("Attraction Matrix");

  const ForceParams &force_params = sim.force_params();
  if (force_params.valid()) {
    const float input_width = 72.0f;
    for (size_t target_class = 0; target_class < class_count; ++target_class) {
      ImGui::PushID(static_cast<int>(target_class));
      float value =
          force_params.attraction[selected_class * class_count + target_class];
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
