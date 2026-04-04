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

/*
 * Simulation owns the particle state and advances it through time.
 *
 * Contract:
 * - Particle data is stored in struct-of-arrays form and exposed by const accessors.
 * - step() mutates positions and velocities in place.
 * - GUI-facing setters update parameters used by the next simulation step.
 */
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

  /*
   * Resizes all particle arrays and initializes class ids deterministically.
   */
  void resize(size_t particles, size_t classes);

  /*
   * Rebuilds the spatial acceleration structure from current particle positions.
   */
  void rebuild_grid();

  /*
   * Enforces the configured world-bound behavior for one particle.
   */
  void apply_bounds(size_t index);

  /*
   * Clamps one particle's velocity to the configured maximum speed.
   */
  void limit_speed(size_t index);

public:
  /*
   * Constructs a simulation with default parameters and the requested sizes.
   * particles and classes define the allocated particle/class capacity.
   */
  Simulation(size_t particles, size_t classes);

  /*
   * Constructs a simulation from explicit parameter blocks.
   * The caller is responsible for passing coherent sizes in params/class data.
   */
  Simulation(const SimulationParams &params,
             const std::vector<ClassParams> &class_params,
             const ForceParams &force_params);

  /*
   * Returns the number of particles owned by the simulation.
   */
  size_t size() const;

  /*
   * Returns the configured number of particle classes.
   */
  size_t class_count() const;

  /*
   * Returns read-only views of particle storage arrays.
   * All returned vectors stay valid for the lifetime of the Simulation object.
   */
  const std::vector<float> &pos_x() const;
  const std::vector<float> &pos_y() const;
  const std::vector<float> &vel_x() const;
  const std::vector<float> &vel_y() const;
  const std::vector<int> &classes() const;

  /*
   * Returns the currently active parameter blocks.
   */
  const SimulationParams &params() const;
  const std::vector<ClassParams> &class_params() const;
  const ForceParams &force_params() const;

  /*
   * Replaces the full force-parameter block.
   * The new values are used by the next simulation step.
   */
  void set_force_params(const ForceParams &force_params);

  /*
   * Replaces the full per-class parameter block.
   * The new values are visible to rendering and future updates immediately.
   */
  void set_class_params(const std::vector<ClassParams> &class_params);

  /*
   * GUI-oriented parameter setters.
   * Each method updates one logical parameter without replacing whole blocks.
   */
  void set_attraction_strength(float strength);
  void set_attraction(size_t source, size_t target, float value);
  void set_class_color(size_t class_index, float r, float g, float b);
  void set_class_mass(size_t class_index, float mass);
  void set_damping(float damping);
  void set_interaction_radius(float radius);
  void set_max_speed(float max_speed);

  /*
   * Writes the full state of one particle.
   * Out-of-range indexes are ignored; class ids are clamped to valid bounds.
   */
  void set_particle(size_t index, float x, float y, float vx, float vy,
                    int particle_class);

  /*
   * Randomizes particle positions, velocities, and class ids.
   * When seed is 0, a non-deterministic seed source is used.
   */
  void randomize(unsigned int seed = 0);

  /*
   * Resets particle positions and clears velocities.
   * Existing class ids are preserved.
   */
  void reset_particles(unsigned int seed = 0);

  /*
   * Advances the simulation by dt seconds.
   * dt <= 0 is ignored.
   */
  void step(float dt);

  /*
   * Advances the simulation by params().time_step seconds.
   */
  void step();
};

#endif
