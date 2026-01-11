#include <external/imgui/imgui.h>
#include "view.hpp"
#include "mini/components/GridRenderer.hpp"
#include "mini/display.hpp"
#include "drawing/GraphRenderer.hpp"
#include <memory>
#include "domain.hpp"
#include <fstream>
#include <filesystem>
#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <mini/components/Camera.hpp>

namespace fs = std::filesystem;

namespace application {

const float toolbarHeight = 50.0f;

class ViewImpl : public View {

  // ========================[APPLICATION UI=============================================
  display::Window* appWindow;

  std::unique_ptr<GridRenderer>  gridRenderer;
  std::shared_ptr<GraphRenderer> graphRenderer;

  GraphRendererConfiguration rendererConfiguration;

  Grid   mainGrid;
  Camera camera;

  float renderScale                 = 1.0f;
  bool  showGenerateMenu            = true;
  bool  showLayoutMenu              = true;
  bool  showGraphStats              = true;
  bool  showExportMenu              = true;
  bool  showRenderConfigurationMenu = false;

  std::shared_ptr<Graph>       currentGraph;
  std::shared_ptr<GraphLayout> currentGraphLayout;

  GeneratorController* currentGenerator;
  LayoutController*    currentLayout;

  struct GraphStatistics {
    size_t    nodeCount       = 0;
    size_t    edgeCount       = 0;
    size_t    nodeCountLayout = 0;
    glm::vec3 centroid        = glm::vec3(0.0f);
    glm::mat4 transform       = glm::mat4(1.0f);
  };

  GraphStatistics stats;

  // Camera related stuff
  bool  panning = false;
  float lastX   = 0.0f;
  float lastY   = 0.0f;

  float dx = 0.0f;
  float dy = 0.0f;

  glm::vec3 camTarget   = glm::vec3(0.0f);
  float     camDistance = 5.0f;
  float     camYaw      = 0.0f;
  float     camPitch    = 0.0f;

  void reloadGraphStats() {
    static Graph* cached = 0;

    if (currentGraph == nullptr) return;
    if (cached == currentGraph.get()) return;

    cached = currentGraph.get();

    stats.nodeCount = currentGraph->getVertexCount();
    stats.edgeCount = currentGraph->getEdgeCount();
  }

  void resetCamera() {
    panning     = false;
    lastX       = 0.0f;
    lastY       = 0.0f;
    dx          = 0.0f;
    dy          = 0.0f;
    camTarget   = glm::vec3(0.0f);
    camDistance = 5.0f;
    camYaw      = 0.0f;
    camPitch    = 0.0f;

    camera.updateMatrices();
  }

  void renderMenu() {
    graphRenderer->configure(rendererConfiguration);
    if (showRenderConfigurationMenu && ImGui::Begin("RenderConfiguration", &showRenderConfigurationMenu)) {

      ImGui::Text("Visualization Options");
      ImGui::SliderFloat("Scale", &renderScale, 0.01f, 10.0f);

      ImGui::Separator();
      ImGui::Text("Meshing Options");
      ImGui::SliderFloat("EdgeLineThickness", &rendererConfiguration.lineThickness, 0.00001f, 0.002f);
      if (currentGraph != nullptr && currentGraphLayout != nullptr && ImGui::Button("Remesh")) {
        graphRenderer->remesh();
      }

      ImGui::Separator();

      ImGui::Text("Rendering info");
      ImGui::Text("Layout centroid: %f %f %f", stats.centroid.x, stats.centroid.y, stats.centroid.z);

      ImGui::Separator();

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

      ImGui::End();
    }
  }

  void exportMenu() {
    if (showExportMenu && ImGui::Begin("Export", &showGraphStats)) {
      if (currentGraph != nullptr && ImGui::Button("Export to disk")) {

        const std::string graphName = "graph";

        fs::path baseDir = fs::path(graphName);

        fs::create_directories(baseDir);

        for (auto* layout : application::getLayouts()) {

          std::vector<glm::vec3> positions = layout->layout(currentGraph)->positions;

          std::string layoutName = layout->getName();
          fs::path    filePath   = baseDir / (layoutName + ".txt");

          std::ofstream file(filePath);
          if (!file.is_open())
            continue;

          for (const glm::vec3& p : positions) {
            file << p.x << " " << p.y << " " << p.z << "\n";
          }

          file.close();
        }
      }
      ImGui::End();
    }
  }

  void reloadLayoutStats() {
    glm::vec3 centroid = glm::vec3(0.0);

    for (int i = 0; i < currentGraphLayout->positions.size(); i++) {
      centroid += currentGraphLayout->positions[i];
    }

    centroid *= 1.0f / (float)currentGraphLayout->positions.size();

    stats.centroid  = centroid;
    stats.transform = glm::translate(glm::mat4(1.0f), -centroid);

    stats.nodeCountLayout = currentGraphLayout->positions.size();
  }

  void statsMenu() {
    if (showGraphStats && ImGui::Begin("Stats", &showGraphStats)) {
      ImGui::Text("Graph stats popup");
      ImGui::Text("Node count: %lu", stats.nodeCount);
      ImGui::Text("Node edge count: %lu", stats.edgeCount);
      ImGui::Separator();
      ImGui::Text("Node count layout: %lu", stats.nodeCountLayout);
      ImGui::End();
    }
  }
  void layoutMenu() {
    if (!showLayoutMenu)
      return;

    if (ImGui::Begin("Layout", &showLayoutMenu)) {

      ImGui::TextUnformatted("Graph Layout");
      ImGui::Separator();

      auto layouts = application::getLayouts();

      static int currentLayoutIndex = 0;

      if (currentLayoutIndex >= layouts.size())
        currentLayoutIndex = 0;

      if (ImGui::BeginCombo(
            "Layout Type",
            layouts[currentLayoutIndex]->getName().c_str())) {

        for (int i = 0; i < layouts.size(); ++i) {
          bool selected = (i == currentLayoutIndex);
          if (ImGui::Selectable(layouts[i]->getName().c_str(), selected)) {
            currentLayoutIndex = i;
            currentLayout      = layouts[i];
          }
          if (selected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      currentLayout = layouts[currentLayoutIndex];

      ImGui::Spacing();
      ImGui::SeparatorText("Parameters");

      if (currentLayout)
        currentLayout->configureUI();

      ImGui::Spacing();
      ImGui::Separator();

      ImGui::BeginDisabled(currentGraph == nullptr);
      if (ImGui::Button("Apply Layout", ImVec2(-1, 0))) {
        currentGraphLayout = currentLayout->layout(currentGraph);
        reloadLayoutStats();
      }
      ImGui::EndDisabled();
    }

    ImGui::End();
  }
  void generateMenu() {

    if (showGenerateMenu && ImGui::Begin("Generate", &showGenerateMenu)) {
      ImGui::Text("Graph generation menu");

      ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
      if (ImGui::BeginTabBar("Generators", tab_bar_flags)) {
        for (auto* gen : application::getGenerators()) {
          if (ImGui::BeginTabItem(gen->getName().c_str())) {
            currentGenerator = gen;
            currentGenerator->configureUI();
            ImGui::EndTabItem();
          }
        }
        ImGui::EndTabBar();

        if (ImGui::Button("Generate")) {
          currentGraph = currentGenerator->generate();
          reloadGraphStats();
        }
      }
      ImGui::End();
    }
  }

  void MainMenuUI() {
    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open...")) {
          std::vector<std::string> files = domain::graphlist();

          for (auto& file : files) {
            std::cout << "> " << file << std::endl;
          }

          if (files.empty()) {
            ImGui::MenuItem("(no graphs found)", nullptr, false, false);
          } else {
            for (const auto& file : files) {
              if (ImGui::MenuItem(file.c_str())) {
                auto graph = domain::graphload(file);
                if (graph) {
                  currentGraph = graph;
                  reloadGraphStats();
                }
              }
            }
          }

          ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Generate")) {
          showGenerateMenu = true;
        }
        if (ImGui::MenuItem("Layout")) {
          showLayoutMenu = true;
        }
        if (ImGui::MenuItem("RenderConfiguration")) {
          showRenderConfigurationMenu = true;
        }
        if (ImGui::MenuItem("Stats")) {
          showGraphStats = true;
        }
        if (ImGui::MenuItem("Save")) {
        }
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }
  }

  void initCamera(Camera& cam) {
    cam.view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  }

  virtual void init(display::Window* window) override {
    this->appWindow = window;

    gridRenderer = std::make_unique<GridRenderer>();

    mainGrid.resolution = 1.0f;
    mainGrid.distance   = 1000.0f;
    mainGrid.color      = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    mainGrid.transform  = glm::mat4(1.0f);

    graphRenderer = createRegularRenderer();
    graphRenderer->init();

    initCamera(camera);
  }

  inline void updateArcballCamera(Camera& cam) {
    camPitch = glm::clamp(camPitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

    glm::vec3 offset;
    offset.x = camDistance * cos(camPitch) * sin(camYaw);
    offset.y = camDistance * sin(camPitch);
    offset.z = camDistance * cos(camPitch) * cos(camYaw);

    glm::vec3 camPos = camTarget + offset;

    cam.view = glm::lookAt(camPos, camTarget, glm::vec3(0, 1, 0));
    cam.updateMatrices();
  }

  inline void panArcballCamera(Camera& cam, float dx, float dy, bool panning, float panSpeed = 0.002f) {
    if (!panning) return;

    glm::vec3 right = glm::normalize(glm::vec3(cam.invView[0]));
    glm::vec3 up    = glm::normalize(glm::vec3(cam.invView[1]));

    glm::vec3 delta = (-right * dx + up * dy) * panSpeed * camDistance;
    camTarget += delta;

    updateArcballCamera(cam);
  }

  inline void zoomArcballCamera(Camera& cam, float zoomDelta, float zoomSpeed = 0.1f) {
    if (zoomDelta == 0.0f) return;

    camDistance = glm::max(0.01f, camDistance - zoomDelta * zoomSpeed);
    updateArcballCamera(cam);
  }

  inline void rotateArcballCamera(Camera& cam, float dx, float dy, bool rotating, float rotateSpeed = 0.005f) {
    if (!rotating) return;

    camYaw += -dx * rotateSpeed;
    camPitch += -dy * rotateSpeed;

    updateArcballCamera(cam);
  }

  void cameraControl(Camera& cam) {
    float aspect = appWindow->getWidth() / appWindow->getHeight();

    cam.proj = glm::perspective(
      glm::radians(60.0f),
      aspect,
      0.1f,
      1000.0f);
    cam.updateMatrices();

    float x = (float)appWindow->getX();
    float y = (float)appWindow->getY();

    dx = x - lastX;
    dy = y - lastY;

    lastX = x;
    lastY = y;

    bool rotating = appWindow->clickOn();
    bool panning  = appWindow->clickOnRight();

    rotateArcballCamera(cam, dx, dy, rotating);
    panArcballCamera(cam, dx, dy, panning);

    float scroll = appWindow->getScroll();
    zoomArcballCamera(cam, scroll);
  }

  virtual void render(float dt) override {
    reloadGraphStats();

    if (!ImGui::GetIO().WantCaptureMouse) {
      cameraControl(camera);
    }

    MainMenuUI();

    if (gridRenderer) {
      gridRenderer->render(mainGrid, camera);
    }

    GraphRendererEntity ent;
    ent.graph  = currentGraph;
    ent.layout = currentGraphLayout;

    if (ent.graph != nullptr && ent.layout != nullptr) {
      graphRenderer->render(ent, camera, glm::scale(glm::mat4(1.0f), glm::vec3(renderScale)) * stats.transform);
    }

    generateMenu();
    layoutMenu();
    statsMenu();
    exportMenu();
    renderMenu();
  }
};


View* createView() {
  return new ViewImpl;
}

} // namespace application
