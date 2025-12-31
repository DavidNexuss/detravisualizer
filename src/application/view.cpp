#include <external/imgui/imgui.h>
#include "view.hpp"
#include "mini/components/Grid.hpp"
#include "mini/components/ArcballCamera.hpp"
#include <memory>

namespace application {

const float toolbarHeight = 50.0f;

class ViewImpl : public View {

  // ========================[APPLICATION UI=============================================

  std::unique_ptr<GridRenderer> gridRenderer;
  Grid                          mainGrid;
  ArcballCamera                 arcballCamera;

  void MainMenuUI() {
    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New")) {
        }
        if (ImGui::MenuItem("Open...")) {
        }
        if (ImGui::MenuItem("Save")) {
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) {
        }
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
        ImGui::MenuItem("Undo", "CTRL+Z");
        ImGui::MenuItem("Redo", "CTRL+Y");
        ImGui::Separator();
        ImGui::MenuItem("Cut", "CTRL+X");
        ImGui::MenuItem("Copy", "CTRL+C");
        ImGui::MenuItem("Paste", "CTRL+V");
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("View")) {
        static bool show_grid  = true;
        static bool show_stats = false;

        ImGui::MenuItem("Grid", nullptr, &show_grid);
        ImGui::MenuItem("Stats", nullptr, &show_stats);
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Help")) {
        ImGui::MenuItem("About");
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }
  }


  //===============[APPLICATION VIEW]============================

  virtual void init() override {
    gridRenderer = std::make_unique<GridRenderer>();

    mainGrid.resolution = 1.0f;
    mainGrid.distance   = 1000.0f;
    mainGrid.color      = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    mainGrid.transform  = glm::mat4(1.0f);

    arcballCamera.lookAlong(glm::vec3(1, 0.5, 0));
  }

  virtual void render(float dt) override {
    MainMenuUI();
    // Render grid
    if (gridRenderer) {
      gridRenderer->render(mainGrid, arcballCamera.getCamera());
    }
  }
};


View* createView() {
  return new ViewImpl;
}

} // namespace application
