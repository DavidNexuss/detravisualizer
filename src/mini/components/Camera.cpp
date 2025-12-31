#include "Camera.hpp"

glm::vec3 Camera::getCameraPosition() const {
  return glm::vec3(invView[3][0], invView[3][1], invView[3][2]);
}
