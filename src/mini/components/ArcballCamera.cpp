#include "ArcballCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>

ArcballCamera::ArcballCamera(
  float            radius,
  const glm::vec3& target,
  float            fovY,
  float            aspect,
  float            zNear,
  float            zFar) :
  m_radius(radius), m_target(target), m_rotation(1, 0, 0, 0), m_fovY(fovY), m_aspect(aspect), m_zNear(zNear), m_zFar(zFar) {}

glm::vec3 ArcballCamera::projectToSphere(float x, float y) const {
  float d = x * x + y * y;
  float z;

  if (d <= 1.0f) {
    z = sqrt(1.0f - d);
  } else {
    float scale = 1.0f / sqrt(d);
    x *= scale;
    y *= scale;
    z = 0.0f;
  }

  return glm::vec3(x, y, z);
}

void ArcballCamera::beginDrag(float xNdc, float yNdc) {
  m_dragging      = true;
  m_lastSpherePos = projectToSphere(xNdc, yNdc);
}

void ArcballCamera::drag(float xNdc, float yNdc) {
  if (!m_dragging) return;

  glm::vec3 curr = projectToSphere(xNdc, yNdc);
  glm::vec3 axis = glm::cross(m_lastSpherePos, curr);
  float     dot  = glm::dot(m_lastSpherePos, curr);

  if (glm::length(axis) > 1e-5f) {
    glm::quat dq(dot, axis.x, axis.y, axis.z);
    m_rotation = glm::normalize(dq * m_rotation);
  }

  m_lastSpherePos = curr;
}

void ArcballCamera::endDrag() {
  m_dragging = false;
}

void ArcballCamera::zoom(float delta) {
  m_radius *= glm::exp(-delta * 0.1f);
}

void ArcballCamera::pan(const glm::vec2& delta) {
  glm::vec3 right = m_rotation * glm::vec3(1, 0, 0);
  glm::vec3 up    = m_rotation * glm::vec3(0, 1, 0);
  m_target += (-right * delta.x + up * delta.y) * m_radius;
}

glm::mat4 ArcballCamera::viewMatrix() const {
  glm::vec3 eye = m_target + m_rotation * glm::vec3(0, 0, m_radius);
  return glm::lookAt(eye, m_target, m_rotation * glm::vec3(0, 1, 0));
}

glm::mat4 ArcballCamera::projectionMatrix() const {
  return glm::perspective(m_fovY, m_aspect, m_zNear, m_zFar);
}
