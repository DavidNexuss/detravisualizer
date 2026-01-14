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
#include <metrics.hpp>
#include <mini/components/Camera.hpp>
#include "graphStats.hpp"
#include "camControls.hpp"

namespace fs = std::filesystem;

namespace application {

const float toolbarHeight = 50.0f;

class ViewImpl : public View {
  float renderScale                 = 1.0f;
  bool  showGenerateMenu            = true;
  bool  showLayoutMenu              = true;
  bool  showGraphStats              = true;
  bool  showExportMenu              = true;
  bool  showRenderConfigurationMenu = false;

  // ========================[APPLICATION UI=============================================

  GraphRendererConfiguration rendererConfiguration;

  display::Window* appWindow;

  CamControls camera;

  std::unique_ptr<GridRenderer>  gridRenderer;
  std::shared_ptr<GraphRenderer> graphRenderer;

  Grid mainGrid;

  std::shared_ptr<Graph>                 currentGraph;
  std::shared_ptr<GraphStatistics>       currentGraphStats;
  std::shared_ptr<GraphLayoutStatistics> currentGrapLayoutStatistics;
  std::shared_ptr<GraphLayout>           currentGraphLayout;

  GeneratorController* currentGenerator = 0;
  LayoutController*    currentLayout    = 0;

  void reloadGraphStats() {
    static Graph* cached = 0;

    if (currentGraph == nullptr) return;
    if (cached == currentGraph.get()) return;

    cached = currentGraph.get();

    currentGraphStats->reload(currentGraph);
  }

  void reloadLayoutStats() {
    currentGrapLayoutStatistics->reload(currentGraph, currentGraphLayout);
  }


  void resetGraph(std::shared_ptr<Graph> graph) {
    this->currentGraph = graph;
  }

  void renderMenu() {
    graphRenderer->configure(rendererConfiguration);
    if (showRenderConfigurationMenu && ImGui::Begin("RenderConfiguration", &showRenderConfigurationMenu)) {

      ImGui::Text("Visualization Options");
      ImGui::SliderFloat("Scale", &renderScale, 0.01f, 10.0f);

      ImGui::Separator();
      ImGui::Text("Meshing Options");
      ImGui::SliderFloat("EdgeLineThickness", &rendererConfiguration.lineThickness, 0.00001f, 0.02f);
      if (currentGraph != nullptr && currentGraphLayout != nullptr && ImGui::Button("Remesh")) {
        graphRenderer->remesh();
      }

      ImGui::Separator();

      ImGui::Text("Rendering info");
      ImGui::Text("Layout centroid: %f %f %f", currentGraphStats->centroid.x, currentGraphStats->centroid.y, currentGraphStats->centroid.z);

      ImGui::Separator();


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

  void statsMenu() {
    if (showGraphStats && ImGui::Begin("Stats", &showGraphStats)) {
      currentGraphStats->ui();
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
    if (!showGenerateMenu)
      return;

    if (ImGui::Begin("Generate", &showGenerateMenu)) {
      ImGui::Text("Graph generation menu");
      ImGui::Separator();

      auto generators = application::getGenerators();

      if (!currentGenerator && !generators.empty()) {
        currentGenerator = generators.front();
      }

      if (ImGui::BeginCombo("Generator", currentGenerator ? currentGenerator->getName().c_str() : "None")) {

        for (auto* gen : generators) {
          bool selected = (gen == currentGenerator);
          if (ImGui::Selectable(gen->getName().c_str(), selected)) {
            currentGenerator = gen;
          }
          if (selected)
            ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      if (currentGenerator) {
        currentGenerator->configureUI();
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      if (ImGui::Button("Generate") && currentGenerator) {
        resetGraph(currentGenerator->generate());
        reloadGraphStats();

        if (currentGraphLayout) {
          currentGraphLayout = currentLayout->layout(currentGraph);
          reloadLayoutStats();
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

  void init(display::Window* window) override {
    this->appWindow = window;

    currentGraphStats           = std::make_shared<GraphStatistics>();
    currentGrapLayoutStatistics = std::make_shared<GraphLayoutStatistics>();
    gridRenderer                = std::make_unique<GridRenderer>();

    mainGrid.resolution = 1.0f;
    mainGrid.distance   = 1000.0f;
    mainGrid.color      = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    mainGrid.transform  = glm::mat4(1.0f);

    graphRenderer = createRegularRenderer();
    graphRenderer->init();

    camera.initCamera();
  }


  virtual void render(float dt) override {
    reloadGraphStats();
    camera.cameraControl(appWindow);

    MainMenuUI();

    if (gridRenderer) {
      gridRenderer->render(mainGrid, camera.cam);
    }

    if (currentGraph && currentLayout && currentLayout->shouldLayout()) {
      currentGraphLayout = currentLayout->layout(currentGraph);
      reloadLayoutStats();
    }

    if (currentGraph && currentLayout && currentLayout->shouldOptimize()) {
      currentLayout->optimize(currentGraph, currentGraphLayout);
      graphRenderer->remesh();
    }

    GraphRendererEntity ent;
    ent.graph  = currentGraph;
    ent.layout = currentGraphLayout;

    if (ent.graph != nullptr && ent.layout != nullptr) {
      graphRenderer->render(ent, camera.cam, glm::scale(glm::mat4(1.0f), glm::vec3(renderScale)) * currentGrapLayoutStatistics->transform);
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
