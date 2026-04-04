/*
 * This module encapsulates the GUI lifecycle and rendering using Dear ImGui
 */

#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <vector>

struct GLFWwindow;
class Simulation;
class ParticleRenderer;

/*
 * GUIManager owns the Dear ImGui context used by the application.
 *
 * Contract:
 * - init() must be called before render() or capture queries are used.
 * - render() mutates simulation-facing state immediately for the next update.
 * - shutdown() releases all ImGui resources and is safe after init().
 */
class GUIManager {
private:
  bool _initialized = false;
  size_t _fpps = 1;

  // State for class editor
  int _selected_class = 0;

  // Timing for step time measurement
  double _last_step_time_ms = 0.0;

  // Pause state
  bool _paused = false;

public:
  /*
   * Constructs a GUI manager in a non-initialized state.
   */
  GUIManager() = default;

  /*
   * Trivial destructor. Lifetime-sensitive cleanup is performed by shutdown().
   */
  ~GUIManager() = default;

  /*
   * Initializes Dear ImGui and its GLFW/OpenGL backends for the given window.
   * Returns false when backend initialization fails.
   */
  bool init(GLFWwindow *window);

  /*
   * Shuts down Dear ImGui and its backends.
   * Safe to call when the manager was never initialized.
   */
  void shutdown();

  /*
   * Builds and renders the full GUI for the current frame.
   * Any user edits are applied directly to sim and are visible next frame.
   */
  void render(Simulation &sim, ParticleRenderer &renderer);

  /*
   * Returns true when Dear ImGui wants to consume mouse input this frame.
   * Callers should suppress camera or world interactions while true.
   */
  bool want_capture_mouse() const;

  /*
   * Returns true when Dear ImGui wants to consume keyboard input this frame.
   * Callers should suppress camera keyboard controls while true.
   */
  bool want_capture_keyboard() const;

  /*
   * Returns whether the GUI currently requests the simulation to be paused.
   */
  bool is_paused() const { return _paused; }

  /*
   * Sets the runtime frames-per-physics-step selector value.
   * Unsupported values are normalized to the nearest supported discrete option.
   */
  void set_fpps(size_t fpps);

  /*
   * Returns the currently selected discrete frames-per-physics-step value.
   */
  size_t fpps() const { return _fpps; }

  /*
   * Updates the diagnostic value displayed for the last simulation step.
   * step_time_ms is interpreted in milliseconds.
   */
  void set_step_time(double step_time_ms);

private:
  /*
   * Renders global simulation controls such as damping, radius, and pause.
   */
  void render_global_params_window(Simulation &sim);

  /*
   * Renders controls for the currently selected particle class.
   */
  void render_class_editor_window(Simulation &sim,
                                  ParticleRenderer &renderer);

  /*
   * Renders the transparent diagnostic overlay shown over the viewport.
   */
  void render_diagnostic_overlay(const Simulation &sim);
};

#endif
