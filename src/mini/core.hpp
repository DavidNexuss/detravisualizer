#pragma once
#include <memory>
namespace minimotor {

struct IApplication {
  virtual void init()         = 0;
  virtual void step(float dt) = 0;
  virtual ~IApplication() {}
};

void run(std::shared_ptr<IApplication> app);
} // namespace minimotor
