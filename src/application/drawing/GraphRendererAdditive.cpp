#include "GraphRenderer.hpp"

//This was planned to serve as an alterntive mode of rendering where nodes are rendered with a point cloud with additive alpha blending,
//however i could not finish and instead i have implemented that very same idea in the egde rendering of the regular algorithm.
struct GraphRendererAdditive : public GraphRenderer {
  ~GraphRendererAdditive() {}

  virtual void init() override {}
  virtual void render(GraphRendererEntity entity, Camera& camera, const glm::mat4& transform) override {}
};
