#pragma once
#include "util.hpp"
#include <memory>
#include <mini/display.hpp>

namespace application {

struct GeneratorController {
  virtual const std::string&     getName()     = 0;
  virtual std::shared_ptr<Graph> generate()    = 0;
  virtual void                   configureUI() = 0;
};

struct LayoutController {
  virtual std::string                  getName()                      = 0;
  virtual void                         configureUI()                  = 0;
  virtual std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph>) = 0;
};

struct View {
  virtual void render(float dt)       = 0;
  virtual void init(display::Window*) = 0;
  virtual ~View() {}
};

View* createView();

std::vector<GeneratorController*> getGenerators();
std::vector<LayoutController*>    getLayouts();
} // namespace application
