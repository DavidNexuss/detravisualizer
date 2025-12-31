#pragma once
namespace application {
struct View {
  virtual void render(float dt) = 0;
  virtual void init()           = 0;
  virtual ~View() {}
};

View* createView();
} // namespace application
