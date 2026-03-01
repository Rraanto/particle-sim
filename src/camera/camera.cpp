#include "camera.h"
#include <algorithm>

void Camera::move(const float &horizontal_stride,
                  const float &vertical_stride) {
  this->_center_x += horizontal_stride * this->_zoom_factor;
  this->_center_y += vertical_stride * this->_zoom_factor;
}

float Camera::get_x() { return this->_center_x; }

float Camera::get_y() { return this->_center_y; }

float Camera::get_zoom() { return this->_zoom_factor; }

float Camera::get_scale() { return this->_scale; }

void Camera::zoom_in(const float &stride) {
  this->_zoom_factor = std::max(
      this->_min_zoom_factor, this->_zoom_factor - stride * this->_zoom_factor);
}

void Camera::zoom_out(const float &stride) {
  this->_zoom_factor = std::min(
      this->_max_zoom_factor, this->_zoom_factor + stride * this->_zoom_factor);
}
