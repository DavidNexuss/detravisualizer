#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <mini/components/Camera.hpp>
#include <imgui/imgui.h>
#include <mini/display.hpp>

struct CamControls {
  bool      panning     = false;
  float     lastX       = 0.0f;
  float     lastY       = 0.0f;
  float     dx          = 0.0f;
  float     dy          = 0.0f;
  glm::vec3 camTarget   = glm::vec3(0.0f);
  float     camDistance = 5.0f;
  float     camYaw      = 0.0f;
  float     camPitch    = 0.0f;

  Camera cam;

  void initCamera();
  void resetCamera();
  void uiConfigure();
  void updateArcballCamera();
  void panArcballCamera(float dx, float dy, bool panning, float panSpeed = 0.002f);
  void zoomArcballCamera(float zoomDelta, float zoomSpeed = 0.1f);
  void rotateArcballCamera(float dx, float dy, bool rotating, float rotateSpeed = 0.005f);
  void cameraControl(display::Window* appWindow);
};
