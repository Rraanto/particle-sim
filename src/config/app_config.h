#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <cmath>
#include <cstddef>
#include <vector>

/*
 * AppConfig is the unified source of truth for all simulation and application parameters.
 * It combines structural setup, global physics, and per-class properties.
 */
struct AppConfig {
  // Structural Parameters (Modifying these typically requires a simulation reset)
  size_t class_count = 1;
  size_t particles_per_class = 1000;
  float grid_cell_size = 0.2f;

  // Global Simulation & Integration
  float interaction_radius = 1.0f;
  float damping = 1.0f;
  float max_speed = 4.0f;
  float time_step = 0.016f;
  bool wrap_bounds = false;
  bool paused = false;
  size_t fpps = 1;

  // Force Global
  float attraction_strength = 0.0f;

  // Per-Class Visual and Physical Properties
  struct ClassData {
    float r = 1.0f, g = 1.0f, b = 1.0f;
    float mass = 1.0f;
  };
  std::vector<ClassData> classes;

  // Attraction Matrix (Flattened: source * class_count + target)
  std::vector<float> attraction_matrix;

  /*
   * Ensures internal vectors match class_count.
   * Preserves existing data in its 2D position; initializes new entries to 1.0f.
   */
  void synchronize_dimensions() {
    if (classes.size() != class_count) {
      classes.resize(class_count, {1.0f, 1.0f, 1.0f, 1.0f});
    }

    const size_t new_size = class_count * class_count;
    if (attraction_matrix.size() != new_size) {
      // Find the old dimension assuming a square matrix
      size_t old_dim = 0;
      if (!attraction_matrix.empty()) {
        old_dim = static_cast<size_t>(std::sqrt(attraction_matrix.size()));
      }

      std::vector<float> new_matrix(new_size, 1.0f);
      const size_t min_dim = (class_count < old_dim) ? class_count : old_dim;

      for (size_t i = 0; i < min_dim; ++i) {
        for (size_t j = 0; j < min_dim; ++j) {
          new_matrix[i * class_count + j] =
              attraction_matrix[i * old_dim + j];
        }
      }
      attraction_matrix = std::move(new_matrix);
    }
  }

  /*
   * Returns total particle count derived from class structure.
   */
  size_t total_particles() const {
    return class_count * particles_per_class;
  }
};

#endif
