#include "mini/core.hpp"
#include "mini/graphics.hpp"
#include "application/ui.hpp"

using namespace minimotor;

struct Application : public IApplication {

  std::unique_ptr<application::UI> ui;

  void init() override {
    ui = std::unique_ptr<application::UI>(application::createUI());
  }

  void step(float dt) override {
    ui->render(dt);

    glClearColor(0.2, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }
};

int main() {
  minimotor::run(std::shared_ptr<IApplication>(new Application));
}
