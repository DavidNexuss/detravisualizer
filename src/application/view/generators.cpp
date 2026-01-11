#include <stdlib.h>
#include <imgui/imgui.h>
#include "generators.hpp"
#include "../view.hpp"

namespace application {
struct GraphErdosRenyi : public GeneratorController {
  std::string name = "ErdosRenyi";

  uint64_t nodeCount   = 100;
  float    probability = 0.05;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::SliderFloat("Probability", &probability, 0.0f, 1.0f);
  }

  std::shared_ptr<Graph> generate() override {
    return std::make_shared<Graph>(
      graphs::generators::erdos_renyi<Graph>(nodeCount, probability));
  }
};

struct GraphBarabasiAlbert : public GeneratorController {
  std::string name = "BarabasiAlbert";

  uint64_t nodeCount    = 100;
  uint64_t initialNodes = 5;
  uint64_t edgesPerNode = 3;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::InputScalar("Initial Nodes", ImGuiDataType_U64, &initialNodes);
    ImGui::InputScalar("Edges / Node", ImGuiDataType_U64, &edgesPerNode);
  }

  std::shared_ptr<Graph> generate() override {
    return std::make_shared<Graph>(
      graphs::generators::barabasi_albert<Graph>(nodeCount, initialNodes, edgesPerNode));
  }
};

struct GraphWattsStrogatz : public GeneratorController {
  std::string name = "WattsStrogatz";

  uint64_t nodeCount = 100;
  uint64_t k         = 4;
  float    beta      = 0.1;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::InputScalar("K", ImGuiDataType_U64, &k);
    ImGui::SliderFloat("Beta", &beta, 0.0f, 1.0f);
  }

  std::shared_ptr<Graph> generate() override {
    return std::make_shared<Graph>(
      graphs::generators::watts_strogatz<Graph>(nodeCount, k, beta));
  }
};

struct GraphPreferentialDirected : public GeneratorController {
  std::string name = "PreferentialDirected";

  uint64_t nodeCount = 100;
  uint64_t edgeCount = 300;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::InputScalar("Edges", ImGuiDataType_U64, &edgeCount);
  }

  std::shared_ptr<Graph> generate() override {
    return std::make_shared<Graph>(
      graphs::generators::preferential_directed<Graph>(nodeCount, edgeCount));
  }
};


struct GraphRecursiveTree : public GeneratorController {
  std::string name = "RecursiveTree";

  uint64_t levels      = 5;
  uint64_t maxPerLevel = 3;
  float    probability = 0.7f;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Levels", ImGuiDataType_U64, &levels);
    ImGui::InputScalar("Max / Level", ImGuiDataType_U64, &maxPerLevel);
    ImGui::SliderFloat("Probability", &probability, 0.0f, 1.0f);
  }

  std::shared_ptr<Graph> generate() override {
    return std::make_shared<Graph>(
      graphs::generators::recursive_tree<Graph>(levels, maxPerLevel, probability));
  }
};

std::vector<GeneratorController*> getGenerators() {
  static GraphErdosRenyi           erdosRenyi;
  static GraphBarabasiAlbert       barabasiAlbert;
  static GraphWattsStrogatz        wattsStrogatz;
  static GraphPreferentialDirected preferentialDirected;
  static GraphRecursiveTree        recursiveTree;

  return {
    &erdosRenyi,
    &barabasiAlbert,
    &wattsStrogatz,
    &preferentialDirected,
    &recursiveTree};
}
} // namespace application
