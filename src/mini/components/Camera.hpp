#pragma once
#include <glm/glm.hpp>

struct Camera {
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 combined;
  glm::mat4 invCombined;
  glm::mat4 invView;

  glm::vec3 getCameraPosition() const;

  void updateMatrices();
};
