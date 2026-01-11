#include "core.hpp"
#include "display.hpp"
#include "graphics.hpp"
#include "ui/styles.hpp"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

namespace minimotor {

void run(std::shared_ptr<IApplication> app) {
  display::init();
  graphics::init();
  display::Window* window = display::windowCreate();
  ui::setupStyle();

  app->init(window);
  while (!window->shouldClose()) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int display_w, display_h;

    display_w = window->getWidth();
    display_h = window->getHeight();

    glViewport(0, 0, display_w, display_h);

    app->step(window->getDeltaTime());

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    window->swapBuffers();
    display::poll();
  }
  display::dispose();
  graphics::dispose();
}
} // namespace minimotor
