#include <stdlib.h>
#include <imgui/imgui.h>
#include "generators.hpp"
#include "../view.hpp"

namespace application {

struct GraphEmpty : public GeneratorController {
  std::string name = "Empty";

  uint64_t nodeCount = 100;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
  }

  std::shared_ptr<Graph> generate() override {
    Graph graph;
    graph.addVertices(nodeCount);
    return std::make_shared<Graph>(std::move(graph));
  }
};
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

struct GraphPowerLawGamma : public GeneratorController {
  std::string name = "PowerLawGamma";

  uint64_t nodeCount    = 100;
  uint64_t initialNodes = 5;
  uint64_t edgesPerNode = 3;

  float  gamma = 3.0;
  double k0    = 1.0;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::InputScalar("Initial Nodes", ImGuiDataType_U64, &initialNodes);
    ImGui::InputScalar("Edges / Node", ImGuiDataType_U64, &edgesPerNode);
    ImGui::SliderFloat("Gamma (γ)", &gamma, 2.00f, 6.0f, "%.2f");
    ImGui::InputDouble("Initial Attractiveness k₀", &k0, 0.1, 1.0);
  }

  std::shared_ptr<Graph> generate() override {
    double alpha = 1.0 / (gamma - 1.0);

    return std::make_shared<Graph>(
      graphs::generators::generalized_preferential_attachment<Graph>(
        nodeCount,
        initialNodes,
        edgesPerNode,
        alpha,
        k0));
  }
};

struct GraphDorogovtsevMendes : public GeneratorController {
  std::string name = "Dorogovtsev–Mendes (Accelerated PA)";

  uint64_t nodeCount    = 100;
  uint64_t initialNodes = 5;
  uint64_t baseEdges    = 1; // m₀ in m(t) = m₀ · t^α

  double gamma = 3.0; // user-facing exponent

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::InputScalar("Initial Nodes", ImGuiDataType_U64, &initialNodes);
    ImGui::InputScalar("Base Edges (m₀)", ImGuiDataType_U64, &baseEdges);

    ImGui::InputDouble("Gamma (γ)", &gamma, 0.1, 0.5);
    ImGui::TextUnformatted("γ > 2.0  (controls power-law exponent)");
  }

  std::shared_ptr<Graph> generate() override {
    double clampedGamma = std::max(2.01, gamma);

    double alpha = 1.0 / (clampedGamma - 1.0);

    alpha = std::clamp(alpha, 0.01, 0.99);

    return std::make_shared<Graph>(
      graphs::generators::dorogovtsev_mendes<Graph>(
        nodeCount,
        initialNodes,
        alpha,
        baseEdges));
  }
};

struct GraphSelectedFitness : public GeneratorController {
  std::string name = "SelectedFitness";

  uint64_t nodeCount = 100;
  float    tau       = 1.0;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::SliderFloat("Tau", &tau, 0.001, 10.0);
  }

  std::shared_ptr<Graph> generate() override {
    return std::make_shared<Graph>(graphs::generators::selected_fitness<Graph>(nodeCount, tau));
  }
};

struct GraphFactorFastSeed : public GeneratorController {
  std::string name = "FactorGraphFastSeed";

  uint64_t nodeCount = 100;
  uint64_t gamma     = 1;
  float    seed      = 1.0f;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::InputScalar("Gamma", ImGuiDataType_U64, &gamma);
    ImGui::InputFloat("Seed", &seed);
  }

  std::shared_ptr<Graph> generate() override {
    return std::make_shared<Graph>(
      graphs::generators::factor_graph_fast_seed<Graph>(nodeCount, gamma, seed));
  }
};

struct GraphFactorFast : public GeneratorController {
  std::string name = "FactorGraphFast";

  uint64_t nodeCount = 100;
  uint64_t gamma     = 1;

  const std::string& getName() override {
    return name;
  }

  void configureUI() override {
    ImGui::InputScalar("Nodes", ImGuiDataType_U64, &nodeCount);
    ImGui::InputScalar("Gamma", ImGuiDataType_U64, &gamma);
  }

  std::shared_ptr<Graph> generate() override {
    return std::make_shared<Graph>(
      graphs::generators::factor_graph_fast<Graph>(nodeCount, gamma));
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
  static GraphFactorFast           factor_graph_fast_seed;
  static GraphFactorFastSeed       factor_graph_fast;
  static GraphEmpty                graphEmpty;
  static GraphDorogovtsevMendes    graphDorogovtsevMendes;
  static GraphSelectedFitness      graphSelectedFitness;
  static GraphPowerLawGamma        graphPowerLawGamma;

  return {
    &erdosRenyi,
    &barabasiAlbert,
    &wattsStrogatz,
    &graphDorogovtsevMendes,
    &graphSelectedFitness,
    &preferentialDirected,
    &recursiveTree,
    &factor_graph_fast_seed,
    &factor_graph_fast,
    &graphEmpty,
    &graphPowerLawGamma};
};

} // namespace application
