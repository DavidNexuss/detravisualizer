#pragma once
#include <application/util.hpp>
#include <mini/components/Camera.hpp>
#include <memory>

struct GraphRendererConfiguration {
  float lineThickness   = 0.0001f;
  bool  colorMode       = false;
  float alphaAmount     = 0.1f;
  bool  confRenderEdges = true;
  bool  confRenderNodes = true;
};

struct GraphRendererEntity {
  std::shared_ptr<Graph>       graph;
  std::shared_ptr<GraphLayout> layout;

  inline bool operator==(const GraphRendererEntity& ent) const {
    return graph == ent.graph && layout == ent.layout;
  }
};

struct GraphRenderer {
  ~GraphRenderer() {}

  virtual void configure(GraphRendererConfiguration)                                          = 0;
  virtual void remesh()                                                                       = 0;
  virtual void init()                                                                         = 0;
  virtual void render(GraphRendererEntity entity, Camera& camera, const glm::mat4& transform) = 0;
};

std::shared_ptr<GraphRenderer> createRegularRenderer();
std::shared_ptr<GraphRenderer> createAdditiveRenderer();
