/*
 * This implements a basic 2D camera
 *
 * Coordinates logic assume unnormalised coordinates
 * (1, 3), (3.14, 83), ... etc
 *
 * HEADLESS
 */
#ifndef CAMERA_H
#define CAMERA_H

/*
 * Camera represents the 2D view transform used by the renderer.
 *
 * Contract:
 * - The camera stores a world-space center, zoom factor, and scale.
 * - move() translates the center in world space.
 * - zoom_in()/zoom_out() keep zoom within the configured min/max bounds.
 */
struct Camera {
private:
  // Minimum and maximum allowed zoom multipliers.
  float _min_zoom_factor = 0.05f;
  float _max_zoom_factor = 20.0f;

  float _center_x, _center_y;
  float _zoom_factor; // The smaller the zoom factor, the closer things appear.
  float _scale;       // Abstract-to-apparent world scale.

public:
  /*
   * Constructs a camera centered at (x, y) with default zoom and scale.
   */
  Camera(const float &x, const float &y)
      : _center_x(x), _center_y(y), _zoom_factor(1.0), _scale(1.0) {}

  /*
   * Returns the current camera center and zoom/scale state.
   */
  float get_x() const;
  float get_y() const;
  float get_zoom() const;
  float get_scale() const;

  /*
   * Translates the camera center in world space.
   * horizontal_stride moves right when positive.
   * vertical_stride moves up when positive.
   */
  void move(const float &horizontal_stride, const float &vertical_stride);

  /*
   * Reduces the zoom factor to zoom in.
   * stride is scaled by the current zoom and then clamped to bounds.
   */
  void zoom_in(const float &stride);

  /*
   * Increases the zoom factor to zoom out.
   * stride is scaled by the current zoom and then clamped to bounds.
   */
  void zoom_out(const float &stride);
};

#endif
