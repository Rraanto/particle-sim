#include "simulation.h"

#include <cmath>
#include <random>

Simulation::Simulation(size_t particles, size_t classes)
    : _params(), _class_params(classes), _force_params() {
  _params.particle_count = particles;
  _params.class_count = classes;
  resize(particles, classes);
  _force_params.class_count = classes;
  _force_params.attraction.assign(classes * classes, 0.0f);
  _grid.reset(_params.grid_cell_size, _params.bounds_min_x,
              _params.bounds_min_y, _params.bounds_max_x, _params.bounds_max_y);
}

Simulation::Simulation(const SimulationParams &params,
                       const std::vector<ClassParams> &class_params,
                       const ForceParams &force_params)
    : _params(params), _class_params(class_params),
      _force_params(force_params) {
  resize(_params.particle_count, _params.class_count);
  _grid.reset(_params.grid_cell_size, _params.bounds_min_x,
              _params.bounds_min_y, _params.bounds_max_x, _params.bounds_max_y);
}

void Simulation::resize(size_t particles, size_t classes) {
  _size = particles;
  _pos_x.assign(particles, 0.0f);
  _pos_y.assign(particles, 0.0f);
  _vel_x.assign(particles, 0.0f);
  _vel_y.assign(particles, 0.0f);
  _classes.assign(particles, 0);

  if (classes == 0) {
    return;
  }
  for (size_t i = 0; i < particles; ++i) {
    _classes[i] = static_cast<int>(i % classes);
  }
}

size_t Simulation::size() const { return _size; }

size_t Simulation::class_count() const { return _params.class_count; }

const std::vector<float> &Simulation::pos_x() const { return _pos_x; }

const std::vector<float> &Simulation::pos_y() const { return _pos_y; }

const std::vector<float> &Simulation::vel_x() const { return _vel_x; }

const std::vector<float> &Simulation::vel_y() const { return _vel_y; }

const std::vector<int> &Simulation::classes() const { return _classes; }

const SimulationParams &Simulation::params() const { return _params; }

const std::vector<ClassParams> &Simulation::class_params() const {
  return _class_params;
}

const ForceParams &Simulation::force_params() const { return _force_params; }

void Simulation::set_force_params(const ForceParams &force_params) {
  _force_params = force_params;
}

void Simulation::set_class_params(
    const std::vector<ClassParams> &class_params) {
  _class_params = class_params;
}

void Simulation::set_particle(size_t index, float x, float y, float vx,
                              float vy, int particle_class) {
  if (index >= _size) {
    return;
  }
  if (_params.class_count > 0) {
    const int max_class = static_cast<int>(_params.class_count);
    if (particle_class < 0) {
      particle_class = 0;
    } else if (particle_class >= max_class) {
      particle_class = max_class - 1;
    }
  }
  _pos_x[index] = x;
  _pos_y[index] = y;
  _vel_x[index] = vx;
  _vel_y[index] = vy;
  _classes[index] = particle_class;
}

void Simulation::randomize(unsigned int seed) {
  if (_size == 0) {
    return;
  }

  std::mt19937 rng;
  if (seed == 0) {
    std::random_device device;
    rng.seed(device());
  } else {
    rng.seed(seed);
  }

  std::uniform_real_distribution<float> pos_x_dist(_params.bounds_min_x,
                                                   _params.bounds_max_x);
  std::uniform_real_distribution<float> pos_y_dist(_params.bounds_min_y,
                                                   _params.bounds_max_y);
  std::uniform_real_distribution<float> vel_dist(-1.0f, 1.0f);

  int max_class = static_cast<int>(_params.class_count);
  if (max_class <= 0) {
    max_class = 1;
  }
  std::uniform_int_distribution<int> class_dist(0, max_class - 1);

  for (size_t i = 0; i < _size; ++i) {
    _pos_x[i] = pos_x_dist(rng);
    _pos_y[i] = pos_y_dist(rng);
    _vel_x[i] = vel_dist(rng);
    _vel_y[i] = vel_dist(rng);
    _classes[i] = class_dist(rng);
  }
}

void Simulation::rebuild_grid() {
  _grid.reset(_params.grid_cell_size, _params.bounds_min_x,
              _params.bounds_min_y, _params.bounds_max_x, _params.bounds_max_y);
  _grid.build(_pos_x, _pos_y);
}

void Simulation::apply_bounds(size_t index) {
  if (_params.wrap_bounds) {
    if (_pos_x[index] < _params.bounds_min_x) {
      _pos_x[index] = _params.bounds_max_x;
    } else if (_pos_x[index] > _params.bounds_max_x) {
      _pos_x[index] = _params.bounds_min_x;
    }

    if (_pos_y[index] < _params.bounds_min_y) {
      _pos_y[index] = _params.bounds_max_y;
    } else if (_pos_y[index] > _params.bounds_max_y) {
      _pos_y[index] = _params.bounds_min_y;
    }
    return;
  }

  if (_pos_x[index] < _params.bounds_min_x) {
    _pos_x[index] = _params.bounds_min_x;
    _vel_x[index] = -_vel_x[index];
  } else if (_pos_x[index] > _params.bounds_max_x) {
    _pos_x[index] = _params.bounds_max_x;
    _vel_x[index] = -_vel_x[index];
  }

  if (_pos_y[index] < _params.bounds_min_y) {
    _pos_y[index] = _params.bounds_min_y;
    _vel_y[index] = -_vel_y[index];
  } else if (_pos_y[index] > _params.bounds_max_y) {
    _pos_y[index] = _params.bounds_max_y;
    _vel_y[index] = -_vel_y[index];
  }
}

void Simulation::limit_speed(size_t index) {
  const float max_speed = _params.max_speed;
  if (max_speed <= 0.0f) {
    return;
  }

  const float vx = _vel_x[index];
  const float vy = _vel_y[index];
  const float speed_sq = vx * vx + vy * vy;
  const float max_speed_sq = max_speed * max_speed;
  if (speed_sq <= max_speed_sq) {
    return;
  }

  const float scale = max_speed / std::sqrt(speed_sq);
  _vel_x[index] = vx * scale;
  _vel_y[index] = vy * scale;
}

void Simulation::step(float dt) {
  if (_size == 0) {
    return;
  }

  if (dt <= 0.0f) {
    return;
  }

  const float radius = _params.interaction_radius;
  const float radius_sq = radius * radius;

  rebuild_grid();

  std::vector<size_t> neighbors;
  neighbors.reserve(64);

  for (size_t i = 0; i < _size; ++i) {
    float ax = 0.0f;
    float ay = 0.0f;

    if (_force_params.valid() && radius > 0.0f) {
      _grid.query(_pos_x[i], _pos_y[i], radius, neighbors);
      for (size_t idx = 0; idx < neighbors.size(); ++idx) {
        const size_t j = neighbors[idx];
        if (j == i) {
          continue;
        }
        const float dx = _pos_x[j] - _pos_x[i];
        const float dy = _pos_y[j] - _pos_y[i];
        const float dist_sq = dx * dx + dy * dy;
        if (dist_sq <= 0.0f || dist_sq > radius_sq) {
          continue;
        }
        const float dist = std::sqrt(dist_sq);
        const float influence = 1.0f - (dist / radius);
        const float force = _force_params.get(static_cast<size_t>(_classes[i]),
                                              static_cast<size_t>(_classes[j]));
        const float inv_dist = 1.0f / dist;
        ax += dx * inv_dist * force * influence;
        ay += dy * inv_dist * force * influence;
      }
    }

    _vel_x[i] += ax * dt;
    _vel_y[i] += ay * dt;

    _vel_x[i] *= _params.damping;
    _vel_y[i] *= _params.damping;

    limit_speed(i);

    _pos_x[i] += _vel_x[i] * dt;
    _pos_y[i] += _vel_y[i] * dt;
    apply_bounds(i);
  }
}

void Simulation::step() { step(_params.time_step); }
