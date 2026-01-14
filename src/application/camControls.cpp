#include "camControls.hpp"

void CamControls::initCamera() {
  cam.view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

void CamControls::resetCamera() {
  panning     = false;
  lastX       = 0.0f;
  lastY       = 0.0f;
  dx          = 0.0f;
  dy          = 0.0f;
  camTarget   = glm::vec3(0.0f);
  camDistance = 5.0f;
  camYaw      = 0.0f;
  camPitch    = 0.0f;

  cam.updateMatrices();
}

void CamControls::uiConfigure() {

  ImGui::Text("Camera State");
  ImGui::Separator();

  ImGui::Text("Target:");
  ImGui::BulletText("X: %.3f", camTarget.x);
  ImGui::BulletText("Y: %.3f", camTarget.y);
  ImGui::BulletText("Z: %.3f", camTarget.z);

  ImGui::Spacing();

  ImGui::Text("Orientation:");
  ImGui::BulletText("Yaw:   %.3f rad", camYaw);
  ImGui::BulletText("Pitch: %.3f rad", camPitch);

  ImGui::Spacing();

  ImGui::Text("Distance:");
  ImGui::BulletText("Radius: %.3f", camDistance);

  ImGui::Spacing();

  ImGui::Text("Input State:");
  ImGui::BulletText("Panning: %s", panning ? "true" : "false");
  ImGui::BulletText("Delta X: %.3f", dx);
  ImGui::BulletText("Delta Y: %.3f", dy);

  ImGui::Separator();

  if (ImGui::Button("Force Reset Camera")) {
    resetCamera();
  }
}

void CamControls::updateArcballCamera() {
  camPitch = glm::clamp(camPitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

  glm::vec3 offset;
  offset.x = camDistance * cos(camPitch) * sin(camYaw);
  offset.y = camDistance * sin(camPitch);
  offset.z = camDistance * cos(camPitch) * cos(camYaw);

  glm::vec3 camPos = camTarget + offset;

  cam.view = glm::lookAt(camPos, camTarget, glm::vec3(0, 1, 0));
  cam.updateMatrices();
}

void CamControls::panArcballCamera(float dx, float dy, bool panning, float panSpeed) {
  if (!panning) return;

  glm::vec3 right = glm::normalize(glm::vec3(cam.invView[0]));
  glm::vec3 up    = glm::normalize(glm::vec3(cam.invView[1]));

  glm::vec3 delta = (-right * dx + up * dy) * panSpeed * camDistance;
  camTarget += delta;

  updateArcballCamera();
}

void CamControls::zoomArcballCamera(float zoomDelta, float zoomSpeed) {
  if (zoomDelta == 0.0f) return;

  camDistance = glm::max(0.01f, camDistance - zoomDelta * zoomSpeed);
  updateArcballCamera();
}

void CamControls::rotateArcballCamera(float dx, float dy, bool rotating, float rotateSpeed) {
  if (!rotating) return;

  camYaw += -dx * rotateSpeed;
  camPitch += -dy * rotateSpeed;

  updateArcballCamera();
}

void CamControls::cameraControl(display::Window* appWindow) {
  float aspect = appWindow->getWidth() / (float)appWindow->getHeight();

  cam.proj = glm::perspective(glm::radians(90.0f), aspect, 0.0005f, 10000.0f);
  cam.updateMatrices();

  float x = (float)appWindow->getX();
  float y = (float)appWindow->getY();

  dx = x - lastX;
  dy = y - lastY;

  lastX = x;
  lastY = y;

  bool rotating = appWindow->clickOn();
  bool panning  = appWindow->clickOnRight();

  if (!ImGui::GetIO().WantCaptureMouse) {
    float scroll = appWindow->getScroll();
    rotateArcballCamera(dx, dy, rotating);
    panArcballCamera(dx, dy, panning);
    zoomArcballCamera(scroll);
  }
}
