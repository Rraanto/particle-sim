#ifndef APP_H
#define APP_H

#include <array>
#include <cstddef>
#include <memory>

#include "cliutils.h"

struct GLFWwindow;
class Camera;
class GUIManager;
class ParticleRenderer;
class Simulation;

/*
 * App owns and orchestrates the full runtime of the program.
 *
 * Contract:
 * - init() must succeed before run() is called.
 * - run() drives the event, update, and render phases until the window closes.
 * - shutdown() is safe to call after partial or complete initialization.
 */
class App {
private:
  /*
   * Action enumerates the logical camera inputs tracked across frames.
   */
  enum class Action {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    ZoomIn,
    ZoomOut,
    Count
  };

  /*
   * InputManager stores callback-driven input state for the current frame.
   * It is intentionally private to keep GLFW routing details inside App.
   */
  struct InputManager {
    std::array<bool, static_cast<size_t>(Action::Count)> active{};
    bool dragging = false;
    double last_x = 0.0;
    double last_y = 0.0;
    double cursor_x = 0.0;
    double cursor_y = 0.0;
    bool cursor_valid = false;
  };

  bool _glfw_initialized = false;
  GLFWwindow *_window = nullptr;
  std::unique_ptr<ParticleRenderer> _renderer;
  std::unique_ptr<GUIManager> _gui_manager;
  std::unique_ptr<Camera> _camera;
  std::unique_ptr<Simulation> _simulation;
  InputManager _input_manager;

  /*
   * Creates the GLFW window and OpenGL context.
   * Returns false when window-system initialization fails.
   */
  bool init_window();

  /*
   * Initializes GPU resources required by the renderer.
   * Requires a valid OpenGL context created by init_window().
   */
  bool init_renderer();

  /*
   * Builds the initial simulation state from the parsed CLI configuration.
   * Returns false when the requested particle/class counts are invalid.
   */
  bool init_simulation(const SimConfig &config);

  /*
   * Connects GLFW callbacks to the static bridge methods below.
   * Requires _window to be valid.
   */
  void register_callbacks();

  /*
   * Runs the event phase: polls GLFW and applies buffered camera input.
   */
  void process_events();

  /*
   * Runs the fixed-timestep update phase.
   * frame_dt is the clamped wall-clock delta for the current frame.
   * accumulator and frame_index are updated in place.
   */
  void update(double frame_dt, double &accumulator, double fixed_dt,
              size_t &frame_index);

  /*
   * Runs the render phase and presents the frame to the window.
   * Requires a valid simulation, camera, renderer, and GUI manager.
   */
  void render();

  /*
   * Applies continuous keyboard-driven camera motion and zoom.
   */
  void apply_input();

  /*
   * Applies drag-based camera panning from the buffered pointer state.
   */
  void apply_mouse_drag();

  /*
   * Returns the App instance associated with a GLFW window user pointer.
   * Returns nullptr when no App instance is registered.
   */
  static App *get_app(GLFWwindow *window);

  /*
   * Reports GLFW-level initialization/runtime errors to stderr.
   */
  static void glfw_error_callback(int error, const char *description);

  /*
   * Static callback bridges required by GLFW.
   * Each wrapper resolves the App instance and forwards to the matching member.
   */
  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);
  static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                    int mods);
  static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
  static void scroll_callback(GLFWwindow *window, double xoffset,
                              double yoffset);
  static void char_callback(GLFWwindow *window, unsigned int codepoint);

  /*
   * Member-side GLFW handlers.
   * These methods update internal input state and forward UI events to ImGui.
   */
  void on_key(int key, int scancode, int action, int mods);
  void on_mouse_button(int button, int action, int mods);
  void on_cursor_pos(double xpos, double ypos);
  void on_scroll(double xoffset, double yoffset);
  void on_char(unsigned int codepoint);

  /*
   * Maps a GLFW key code to a logical Action.
   * Returns false when the key does not drive camera input.
   */
  bool map_key_to_action(int key, Action &out_action) const;

public:
  /*
   * Constructs an App in a non-initialized state.
   */
  App();

  /*
   * Destroys the App object.
   * The implementation supports incomplete types and does not guarantee
   * shutdown().
   */
  ~App();

  /*
   * Initializes all owned subsystems from CLI configuration.
   * Returns true only when windowing, rendering, GUI, and simulation are ready.
   */
  bool init(const SimConfig &config);

  /*
   * Enters the main loop until the window requests closure.
   * Calling run() before a successful init() is a no-op.
   */
  void run();

  /*
   * Tears down all owned systems in dependency-safe order.
   * Safe to call multiple times and after failed initialization.
   */
  void shutdown();
};

#endif
