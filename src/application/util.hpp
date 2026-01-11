#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <graph.hpp>
#include <graphbackend/adjacencylist.hpp>
#include <position.hpp>

struct DataSet {
  std::vector<float> x;
  std::vector<float> y;
  bool               scale;
};

using GraphLayout = graphs::position::PositionTable;
using Graph       = graphs::Graph<graphs::backends::AdjacencyListVector>;
