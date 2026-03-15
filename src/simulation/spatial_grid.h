/*
 * This library handles the grid-based organisation and queries of particles
 (neighbor searches etc...)
 */

#ifndef SPATIAL_GRID_H
#define SPATIAL_GRID_H

#include <cstddef>
#include <vector>

class SpatialGrid {
public:
  SpatialGrid() = default;
  SpatialGrid(float cell_size, float min_x, float min_y, float max_x,
              float max_y);

  void reset(float cell_size, float min_x, float min_y, float max_x,
             float max_y);

  void build(const std::vector<float> &pos_x, const std::vector<float> &pos_y);

  /*
   * Returns all the (indexes of) particles that are within a radius
   * <radius> of the (x, y) point
   */
  void query(float x, float y, float radius,
             std::vector<size_t> &out_indices) const;

  /*
   * Checks if instanciated grid (_cols and _rows) is valid
   */
  bool valid() const;

private:
  float _cell_size = 0.0f;
  float _inv_cell_size = 0.0f;
  float _min_x = 0.0f;
  float _min_y = 0.0f;
  float _max_x = 0.0f;
  float _max_y = 0.0f;

  size_t _cols = 0;
  size_t _rows = 0;

  std::vector<std::vector<size_t>> _cells;

  size_t cell_index(size_t cx, size_t cy) const;
  size_t clamp_cell(long value, size_t upper) const;
};

#endif
