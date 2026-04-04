/*
 * This libary defines structs for sets of parameters:
 * - for overall simulation (number of particles etc...)
 * - classes: colors per particle class
 * - force: force interactions
 */
#ifndef PARAMS_H
#define PARAMS_H

#include <cstddef>
#include <vector>

/*
 * SimulationParams groups global physical and numerical settings.
 */
struct SimulationParams {
  // Declared particle/class counts used to size the simulation.
  size_t particle_count = 0;
  size_t class_count = 0;

  // World bounds used for wrapping or collision against borders.
  float bounds_min_x = -1.0f;
  float bounds_min_y = -1.0f;
  float bounds_max_x = 1.0f;
  float bounds_max_y = 1.0f;

  // Integration parameters.
  float time_step = 0.016f;
  float damping = 1.0f;
  float max_speed = 4.0f;

  // Neighborhood search parameters.
  float interaction_radius = 1.0f;
  float grid_cell_size = 0.2f;

  // When true, particles wrap across bounds instead of bouncing.
  bool wrap_bounds = true;
};

/*
 * ClassParams describes the visual and mass properties of one particle class.
 */
struct ClassParams {
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float mass = 1.0f;
};

/*
 * ForceParams stores the attraction matrix shared by all particles.
 * attraction is stored row-major as source_class * class_count + target_class.
 */
struct ForceParams {
  size_t class_count = 0;
  float strength = 1.0f;
  std::vector<float> attraction;

  /*
   * Returns true when the attraction matrix shape matches class_count.
   */
  bool valid() const {
    return class_count > 0 && attraction.size() == class_count * class_count;
  }

  /*
   * Returns the effective interaction value including global strength scaling.
   * Invalid indices return 0.0f.
   */
  float get(size_t source, size_t target) const {
    if (!valid() || source >= class_count || target >= class_count) {
      return 0.0f;
    }
    return attraction[source * class_count + target] * strength;
  }

  /*
   * Writes one raw attraction-matrix entry.
   * Out-of-range indices are ignored.
   */
  void set(size_t source, size_t target, float value) {
    if (source >= class_count || target >= class_count) {
      return;
    }
    if (attraction.size() != class_count * class_count) {
      attraction.assign(class_count * class_count, 0.0f);
    }
    attraction[source * class_count + target] = value;
  }
};

#endif
