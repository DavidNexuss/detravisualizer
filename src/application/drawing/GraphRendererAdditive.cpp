#include "GraphRenderer.hpp"

struct GraphRendererAdditive : public GraphRenderer {
  ~GraphRendererAdditive() {}

  virtual void init() override {}
  virtual void render(GraphRendererEntity entity, Camera& camera, const glm::mat4& transform) override {}
};
