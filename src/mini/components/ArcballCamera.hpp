#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Camera.hpp"

struct ArcballCamera {
  ArcballCamera(float radius, const glm::vec3& target, float fovY, float aspect, float zNear, float zFar);

  void beginDrag(float xNdc, float yNdc);
  void drag(float xNdc, float yNdc);
  void endDrag();

  void zoom(float delta);
  void pan(const glm::vec2& delta);

  Camera getCamera() const;

  glm::vec3 projectToSphere(float x, float y) const;

  float     m_radius;
  glm::vec3 m_target;
  glm::quat m_rotation;

  float m_fovY, m_aspect, m_zNear, m_zFar;

  glm::mat4 viewMatrix() const;
  glm::mat4 projectionMatrix() const;

  bool      m_dragging = false;
  glm::vec3 m_lastSpherePos;
};
