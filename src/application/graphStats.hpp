#pragma once
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include "util.hpp"

struct GraphStatistics {
  size_t                nodeCount       = 0;
  size_t                edgeCount       = 0;
  size_t                nodeCountLayout = 0;
  glm::vec3             centroid        = glm::vec3(0.0f);
  glm::mat4             transform       = glm::mat4(1.0f);
  std::vector<uint32_t> nodeDegree;
  std::vector<uint32_t> nodeDegreeSorted;
  std::vector<uint32_t> nodeDegreeIota;

  std::vector<float> nodeDegreeSortedLog;
  std::vector<float> nodeDegreeIotaLog;

  void reload(std::shared_ptr<Graph> graph);
  void ui();
};

struct GraphLayoutStatistics {
  glm::vec3 centroid  = glm::vec3(0.0f);
  glm::mat4 transform = glm::mat4(1.0f);

  void reload(std::shared_ptr<Graph> graph, std::shared_ptr<GraphLayout> layout);
};
