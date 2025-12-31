#pragma once
#include <glm/glm.hpp>
#include "Camera.hpp"

struct Grid {
  float     resolution = 1.0f;
  float     distance   = 1000.0f;
  glm::vec4 color      = glm::vec4(1.0f);
  glm::mat4 transform  = glm::mat4(1.0f);
};

class GridRenderer {
  public:
  GridRenderer();
  ~GridRenderer();

  GridRenderer(const GridRenderer&)            = delete;
  GridRenderer& operator=(const GridRenderer&) = delete;

  void render(const Grid& grid, const Camera& camera);

  private:
  void init();
  void initVao();
  void initProgram();
  void destroy();

  private:
  unsigned int m_program;
  unsigned int m_vao;
  unsigned int m_vbo;

  int uInvViewProj;
  int uCameraPos;
  int uGridSpacing;
  int uLineRadius;
  int uGridColor;
  int uBackgroundColor;
  int uTransform;
};
