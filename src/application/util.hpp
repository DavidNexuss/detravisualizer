#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <graph.hpp>
#include <graphbackend/adjacencylist.hpp>
#include <position.hpp>
#include <memory>

struct DataSet {
  std::vector<float> x;
  std::vector<float> y;
  bool               scale;
};

using GraphLayout = graphs::position::PositionTable;
using Graph       = graphs::Graph<graphs::backends::AdjacencyListVector>;


struct Conllu {
  std::shared_ptr<Graph>                                                  graph;
  std::shared_ptr<std::unordered_map<uint32_t, std::vector<std::string>>> labels;
};
