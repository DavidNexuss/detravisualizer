#include "mini/core.hpp"
#include "mini/graphics.hpp"
#include "application/view.hpp"
#include "mini/display.hpp"

using namespace minimotor;

struct Application : public IApplication {

  std::unique_ptr<application::View> ui;

  void init(display::Window* window) override {
    ui = std::unique_ptr<application::View>(application::createView());
    ui->init(window);

    glEnable(GL_MULTISAMPLE);
  }

  void step(float dt) override {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ui->render(dt);
  }
};

int main() {
  minimotor::run(std::shared_ptr<IApplication>(new Application));
}
