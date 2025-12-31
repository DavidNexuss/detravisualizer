#pragma once
namespace application {
struct UI {
  virtual void render(float dt) = 0;

  virtual ~UI() {}
};

UI* createUI();
} // namespace application
