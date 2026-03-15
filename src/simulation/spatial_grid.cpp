#include "spatial_grid.h"

#include <cmath>

SpatialGrid::SpatialGrid(float cell_size, float min_x, float min_y, float max_x,
                         float max_y) {
  reset(cell_size, min_x, min_y, max_x, max_y);
}

void SpatialGrid::reset(float cell_size, float min_x, float min_y, float max_x,
                        float max_y) {

  _cell_size = cell_size;
  _min_x = min_x;
  _min_y = min_y;
  _max_x = max_x;
  _max_y = max_y;

  // clamp parameters to valid bounds
  if (_cell_size <= 0.0f || _max_x <= _min_x || _max_y <= _min_y) {
    _cell_size = 0.0f;
    _inv_cell_size = 0.0f;
    _cols = 0;
    _rows = 0;
    _cells.clear();
    return;
  }

  _inv_cell_size = 1.0f / _cell_size;
  const float width = _max_x - _min_x;
  const float height = _max_y - _min_y;

  _cols = static_cast<size_t>(std::ceil(width * _inv_cell_size));
  _rows = static_cast<size_t>(std::ceil(height * _inv_cell_size));

  if (_cols == 0) {
    _cols = 1;
  }
  if (_rows == 0) {
    _rows = 1;
  }

  _cells.assign(_cols * _rows, std::vector<size_t>{});
}

// checks if instanciated grid is usable
bool SpatialGrid::valid() const { return _cols > 0 && _rows > 0; }

size_t SpatialGrid::cell_index(size_t cx, size_t cy) const {
  return cy * _cols + cx;
}

size_t SpatialGrid::clamp_cell(long value, size_t upper) const {
  if (upper == 0) {
    return 0;
  }
  if (value < 0) {
    return 0;
  }
  const long max_value = static_cast<long>(upper - 1);
  if (value > max_value) {
    return upper - 1;
  }
  return static_cast<size_t>(value);
}

void SpatialGrid::build(const std::vector<float> &pos_x,
                        const std::vector<float> &pos_y) {
  if (!valid() || pos_x.size() != pos_y.size()) {
    return;
  }

  for (auto &cell : _cells) {
    cell.clear();
  }

  const size_t count = pos_x.size();
  for (size_t i = 0; i < count; ++i) {
    const float x = pos_x[i];
    const float y = pos_y[i];
    const long cx =
        static_cast<long>(std::floor((x - _min_x) * _inv_cell_size));
    const long cy =
        static_cast<long>(std::floor((y - _min_y) * _inv_cell_size));
    const size_t clamped_x = clamp_cell(cx, _cols);
    const size_t clamped_y = clamp_cell(cy, _rows);
    _cells[cell_index(clamped_x, clamped_y)].push_back(i);
  }
}

void SpatialGrid::query(float x, float y, float radius,
                        std::vector<size_t> &out_indices) const {
  out_indices.clear();
  if (!valid() || radius <= 0.0f) {
    return;
  }

  const float min_x = x - radius;
  const float min_y = y - radius;
  const float max_x = x + radius;
  const float max_y = y + radius;

  const long min_cx =
      static_cast<long>(std::floor((min_x - _min_x) * _inv_cell_size));
  const long min_cy =
      static_cast<long>(std::floor((min_y - _min_y) * _inv_cell_size));
  const long max_cx =
      static_cast<long>(std::floor((max_x - _min_x) * _inv_cell_size));
  const long max_cy =
      static_cast<long>(std::floor((max_y - _min_y) * _inv_cell_size));

  // define bounds of relevant grid
  const size_t start_x = clamp_cell(min_cx, _cols);
  const size_t start_y = clamp_cell(min_cy, _rows);
  const size_t end_x = clamp_cell(max_cx, _cols);
  const size_t end_y = clamp_cell(max_cy, _rows);

  if (start_x > end_x || start_y > end_y) {
    return;
  }

  // search within relevant grid
  for (size_t cy = start_y; cy <= end_y; ++cy) {
    for (size_t cx = start_x; cx <= end_x; ++cx) {
      const auto &cell = _cells[cell_index(cx, cy)];
      out_indices.insert(out_indices.end(), cell.begin(), cell.end());
    }
  }
}
