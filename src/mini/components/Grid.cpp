#include "Grid.hpp"
#include <glad/glad.h>
#include <mini/graphics.hpp>
#include <mini/io.hpp>

const char* fsPath = "assets/shaders/grid.frag";
const char* vsPath = "assets/shaders/grid.vert";

GridRenderer::GridRenderer() : m_program(0), m_vao(0) {
  init();
}

GridRenderer::~GridRenderer() {
  destroy();
}

void GridRenderer::initVao() {
  const float quadVertices[] = {
    // x     y     z
    -1.0f, 0.0f, -1.0f,
    1.0f, 0.0f, -1.0f,
    1.0f, 0.0f, 1.0f,

    -1.0f, 0.0f, -1.0f,
    1.0f, 0.0f, 1.0f,
    -1.0f, 0.0f, 1.0f};

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0,
    3,
    GL_FLOAT,
    GL_FALSE,
    3 * sizeof(float),
    (void*)0);

  glBindVertexArray(0);
}

void GridRenderer::initProgram() {
  m_program = graphics::loadProgram(fsPath, vsPath);

  uInvViewProj     = glGetUniformLocation(m_program, "invViewProj");
  uCameraPos       = glGetUniformLocation(m_program, "cameraPos");
  uGridSpacing     = glGetUniformLocation(m_program, "gridSpacing");
  uLineRadius      = glGetUniformLocation(m_program, "lineRadius");
  uGridColor       = glGetUniformLocation(m_program, "gridColor");
  uBackgroundColor = glGetUniformLocation(m_program, "backgroundColor");
  uTransform       = glGetUniformLocation(m_program, "u_Transform");
}

void GridRenderer::init() {
  initProgram();
  initVao();
}

void GridRenderer::destroy() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
  glDeleteProgram(m_program);
}

void GridRenderer::render(const Grid& grid, const Camera& camera) {

  if (io::fileHasChanged(fsPath) || io::fileHasChanged(vsPath)) {
    try {
      initProgram();
    } catch (const std::exception& e) {
      fprintf(stderr, "Failed to reload grid shader: %s\n", e.what());
      // continue using previous shader
    }
  }
  glUseProgram(m_program);
  glBindVertexArray(m_vao);

  glm::vec3 campos = camera.getCameraPosition();

  glUniformMatrix4fv(uInvViewProj, 1, GL_FALSE, &camera.invCombined[0][0]);
  glUniform3fv(uCameraPos, 1, &campos[0]);
  glUniform1f(uGridSpacing, grid.resolution);
  glUniform1f(uLineRadius, grid.distance);
  glUniform3fv(uGridColor, 1, &glm::vec3(grid.color)[0]);
  glUniform3fv(uBackgroundColor, 1, &glm::vec3(0.0f)[0]);
  glUniformMatrix4fv(uTransform, 1, GL_FALSE, &grid.transform[0][0]);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}
