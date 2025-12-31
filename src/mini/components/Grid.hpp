
#pragma once
#include <glm/glm.hpp>
#include "Camera.hpp"

class Grid {
  public:
  Grid();
  ~Grid();

  void setResolution(float res);
  void setDistance(float distance);
  void setColorMultiplier(const glm::vec4& color);
  void setTransform(const glm::mat4& transform);
  void render(const Camera& camera);
  void gridController();

  private:
  unsigned int m_program = 0;
  unsigned int m_vao     = 0;

  float     m_resolution = 1.0f;
  float     m_distance   = 1000.0f;
  glm::vec4 m_color      = glm::vec4(1.0f);
  glm::mat4 m_transform  = glm::mat4(1.0f);
};
