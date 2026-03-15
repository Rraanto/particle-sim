/*
 * This library is headless
 * and defines the structures needed to define a simulation state
 */
#ifndef SIMULATION_H
#define SIMULATION_H

#include <cstddef>
#include <vector>

#include "spatial_grid.h"
#include "params.h"

class Simulation {
private:
  size_t _size = 0; // size of simulation (number of particles)

  SimulationParams _params;
  std::vector<ClassParams> _class_params;
  ForceParams _force_params;

  SpatialGrid _grid;

  // Struct of arrays: memory-side information about particles
  std::vector<float> _pos_x;
  std::vector<float> _pos_y;
  std::vector<float> _vel_x;
  std::vector<float> _vel_y;
  std::vector<int> _classes;

  void resize(size_t particles, size_t classes);
  void rebuild_grid();
  void apply_bounds(size_t index);
  void limit_speed(size_t index);

public:
  /*
   * Initialises a simulation
   * particles, classes: number of particles and classes
   */
  Simulation(size_t particles, size_t classes);

  Simulation(const SimulationParams &params,
             const std::vector<ClassParams> &class_params,
             const ForceParams &force_params);

  size_t size() const;
  size_t class_count() const;

  const std::vector<float> &pos_x() const;
  const std::vector<float> &pos_y() const;
  const std::vector<float> &vel_x() const;
  const std::vector<float> &vel_y() const;
  const std::vector<int> &classes() const;

  const SimulationParams &params() const;
  const std::vector<ClassParams> &class_params() const;
  const ForceParams &force_params() const;

  void set_force_params(const ForceParams &force_params);
  void set_class_params(const std::vector<ClassParams> &class_params);

  /*
   * Sets attributes to particle at index <index>
   * x, y: position
   * vx, vy: velocity
   * particle_class: class
   */
  void set_particle(size_t index, float x, float y, float vx, float vy,
                    int particle_class);
  void randomize(unsigned int seed = 0);

  void step(float dt);
  void step();
};

#endif
