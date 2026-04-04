/*
 * This library handles the grid-based organisation and queries of particles
 (neighbor searches etc...)
 */

#ifndef SPATIAL_GRID_H
#define SPATIAL_GRID_H

#include <cstddef>
#include <vector>

/*
 * SpatialGrid accelerates neighborhood queries for particle interactions.
 *
 * Contract:
 * - reset() defines the grid geometry and clears previous cell contents.
 * - build() must be called after positions change and before query().
 * - query() appends matching particle indices into the caller-provided vector.
 */
class SpatialGrid {
public:
  /*
   * Constructs an empty, invalid grid.
   */
  SpatialGrid() = default;

  /*
   * Constructs and initializes a grid covering the provided bounds.
   */
  SpatialGrid(float cell_size, float min_x, float min_y, float max_x,
              float max_y);

  /*
   * Reinitializes the grid dimensions and clears all cell contents.
   * cell_size must be positive for the grid to become valid.
   */
  void reset(float cell_size, float min_x, float min_y, float max_x,
             float max_y);

  /*
   * Rebuilds cell membership from particle positions.
   * pos_x and pos_y are expected to describe the same particle ordering.
   */
  void build(const std::vector<float> &pos_x, const std::vector<float> &pos_y);

  /*
   * Appends indices of particles near (x, y) within the queried disk.
   * out_indices is treated as output storage owned by the caller.
   */
  void query(float x, float y, float radius,
             std::vector<size_t> &out_indices) const;

  /*
   * Returns whether the grid dimensions and cell size describe a usable grid.
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

  /*
   * Converts integer cell coordinates to a flat cell index.
   * The caller must pass coordinates already clamped to grid bounds.
   */
  size_t cell_index(size_t cx, size_t cy) const;

  /*
   * Clamps a signed cell coordinate to the inclusive grid range [0, upper - 1].
   */
  size_t clamp_cell(long value, size_t upper) const;
};

#endif
