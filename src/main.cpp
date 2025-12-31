#include "mini/core.hpp"
#include "mini/graphics.hpp"
#include "application/view.hpp"

using namespace minimotor;

struct Application : public IApplication {

  std::unique_ptr<application::View> ui;

  void init() override {
    ui = std::unique_ptr<application::View>(application::createView());
    ui->init();
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
