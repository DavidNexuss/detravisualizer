#include <external/imgui/imgui.h>
#include "ui.hpp"
#include "mini/components/Grid.hpp"
#include "mini/components/ArcballCamera.hpp"
#include <memory>

namespace application {

const float toolbarHeight = 50.0f;

class UIImpl : public UI {

  // ========================[APPLICATION UI=============================================

  std::unique_ptr<Grid>          grid;
  std::unique_ptr<ArcballCamera> arcballCamera;

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

  virtual void init() {
    grid = std::make_unique<Grid>();
  }

  virtual void render(float dt) {
    MainMenuUI();
  }
};


UI* createUI() {
  return new UIImpl;
}

} // namespace application
