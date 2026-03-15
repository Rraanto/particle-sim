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

struct SimulationParams {
  size_t particle_count = 0;
  size_t class_count = 0;

  float bounds_min_x = -1.0f;
  float bounds_min_y = -1.0f;
  float bounds_max_x = 1.0f;
  float bounds_max_y = 1.0f;

  float time_step = 0.016f;
  float damping = 0.98f;
  float max_speed = 4.0f;

  float interaction_radius = 0.2f;
  float grid_cell_size = 0.2f;

  bool wrap_bounds = false;
};

struct ClassParams {
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float mass = 1.0f;
};

struct ForceParams {
  size_t class_count = 0;
  float strength = 1.0f;
  std::vector<float> attraction;

  bool valid() const {
    return class_count > 0 && attraction.size() == class_count * class_count;
  }

  float get(size_t source, size_t target) const {
    if (!valid() || source >= class_count || target >= class_count) {
      return 0.0f;
    }
    return attraction[source * class_count + target] * strength;
  }

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
