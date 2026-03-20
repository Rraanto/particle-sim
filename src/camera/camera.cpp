#include "camera.h"
#include <algorithm>

void Camera::move(const float &horizontal_stride,
                  const float &vertical_stride) {
  this->_center_x += horizontal_stride;
  this->_center_y += vertical_stride;
}

float Camera::get_x() const { return this->_center_x; }

float Camera::get_y() const { return this->_center_y; }

float Camera::get_zoom() const { return this->_zoom_factor; }

float Camera::get_scale() const { return this->_scale; }

void Camera::zoom_in(const float &stride) {
  this->_zoom_factor = std::max(
      this->_min_zoom_factor, this->_zoom_factor - stride * this->_zoom_factor);
}

void Camera::zoom_out(const float &stride) {
  this->_zoom_factor = std::min(
      this->_max_zoom_factor, this->_zoom_factor + stride * this->_zoom_factor);
}
