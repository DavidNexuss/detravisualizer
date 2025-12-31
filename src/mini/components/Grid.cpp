#include "Grid.hpp"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <mini/graphics.hpp>

const static char* fsPath = "assets/shaders/grid.frag";
const static char* vsPath = "assets/shaders/grid.vert";

struct GridUniformLocations {
  GLuint u_invViewProj;
  GLuint u_cameraPos;
  GLuint u_gridSpacing;
  GLuint u_maxDistance;
  GLuint u_colorMul;
};

Grid::Grid() {
  m_program = graphics::loadProgram(fsPath, vsPath);
  glGenVertexArrays(1, &m_vao);
}

Grid::~Grid() {
  if (m_program) glDeleteProgram(m_program);
  if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Grid::setResolution(float res) {
  m_resolution = res;
}

void Grid::setDistance(float distance) {
  m_distance = distance;
}

void Grid::setColorMultiplier(const glm::vec4& colorMultiplier) {
  m_color = colorMultiplier;
}

void Grid::setTransform(const glm::mat4& transform) {
  m_transform = transform;
}

void Grid::gridController() {
}

void Grid::render(const Camera& camera) {
  glUseProgram(m_program);
  glBindVertexArray(m_vao);

  glm::mat4 invVP = camera.invCombined;

  glm::vec3 cameraPosition = camera.getCameraPosition();

  glUniformMatrix4fv(glGetUniformLocation(m_program, "invViewProj"), 1, GL_FALSE, &invVP[0][0]);
  glUniform3fv(glGetUniformLocation(m_program, "cameraPos"), 1, &cameraPosition.x);
  glUniform1f(glGetUniformLocation(m_program, "gridSpacing"), m_resolution);
  glUniform1f(glGetUniformLocation(m_program, "maxDistance"), m_distance);
  glUniform4fv(glGetUniformLocation(m_program, "colorMul"), 1, &m_color[0]);

  glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glEnable(GL_DEPTH_TEST);
}
