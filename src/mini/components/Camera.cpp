#include "Camera.hpp"

glm::vec3 Camera::getCameraPosition() const {
  return glm::vec3(invView[3]);
}

void Camera::updateMatrices() {
  combined = proj * view;

  invView     = glm::inverse(view);
  invCombined = glm::inverse(combined);
}
