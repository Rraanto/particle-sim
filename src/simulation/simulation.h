/*
 * This library is headless
 * and defines the structures needed to define a simulation state
 */
#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>

#include "spatial_grid.h"
#include "params.h"

class Simulation {
private:
  size_t _size; // size of simulation (number of particles)

  /*
   * Struct of arrays:
   * These arrays stores memory-side information about particles within a
   * simulation state
   */
  // position vectors
  std::vector<float> _pos_x;
  std::vector<float> _pos_y;

  // velocity vectors
  std::vector<float> _vel_x;
  std::vector<float> _vel_y;

  // classes of particles
  std::vector<int> _classes;

  std::vector<std::pair<double, double>> _spatial_grid;

public:
  /*
   * Initialises a simulation
   * particles, classes: number of particles and classes
   */
  Simulation(size_t particles, size_t classes)
      : _pos_x(particles), _pos_y(particles), _vel_x(particles),
        _vel_y(particles), _classes(classes) {}
};

#endif
