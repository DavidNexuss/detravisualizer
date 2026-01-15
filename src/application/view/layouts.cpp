#include <position/kamada.hpp>
#include <position/stressMajorization.hpp>
#include <position/3dshell.hpp>
#include <position/degreeOrderGrowth.hpp>
#include <position/linlog.hpp>
#include <position/radialPlacement.hpp>
#include <position/spectralCPU.hpp>
#include <position/spring.hpp>
#include <position/furchtermanReingold.hpp>
#include <application/experiment/algorithmA.hpp>
#include <application/experiment/sunflower.hpp>
#include <imgui/imgui.h>
#include <external/imbezier/curve.hpp>
#include "../view.hpp"

namespace application {
using namespace graphs::position;

/**
 * This is a trick to reset the table automatically if the layout or the graph is changed across executions
 * so we can reuse the table whenever is needed for the sequential execution algorithms
 */
struct PositionTableCache {
  std::shared_ptr<PositionTable> positions;
  Graph*                         cachedGraph;
  LayoutController*              cachedLayout;

  PositionTableCache() {
    positions = std::make_shared<PositionTable>();
  }

  std::shared_ptr<PositionTable> getPositionTable(std::shared_ptr<Graph> graph, LayoutController* layout) {
    if (positions == nullptr) positions = std::make_shared<PositionTable>();
    if (cachedGraph != graph.get()) { positions->positions.clear(); }
    if (cachedLayout != layout) { positions->positions.clear(); }
    return positions;
  }

  void reset() {
    positions = {};
  }
};

PositionTableCache ptableCache;

struct sunflowerGUI : LayoutController {

  int   depth    = 7;
  int   breath   = 3;
  int   maxN     = 1000;
  float distance = 20.0f;
  float gamma    = 1.2f;

  sunflowerGUI() {
    depth    = 7;
    breath   = 3;
    maxN     = 1000;
    distance = 20.0f;
    gamma    = 1.2f;
  }
  void configureUI() override {
    ImGui::SeparatorText("sunflowerGUI");

    ImGui::InputInt("Depth", &depth);
    ImGui::InputInt("Breath", &breath);
    ImGui::InputInt("Max N", &maxN);
    ImGui::InputFloat("Distance", &distance);
    ImGui::InputFloat("Gamma", &gamma);

    depth  = std::max(0, depth);
    breath = std::max(0, breath);
    maxN   = std::max(0, maxN);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    ptableCache.reset();

    auto layout = ptableCache.getPositionTable(graph, this);

    layout->positions = sunflower(depth, breath, maxN, distance, gamma);
    layout->positions.resize(graph->getVertexCount());

    return layout;
  }

  std::string getName() override {
    return "sunflowerGUI";
  }
};

struct Shell3DCreateInfoGUI : public graphs::position::Shell3DCreateInfo, LayoutController {

  void configureUI() override {
    ImGui::SeparatorText("Shell 3D");

    ImGui::InputScalar("Source Vertex", ImGuiDataType_U64, &source);
    ImGui::InputFloat("Layer Spacing", &layerSpacing);
    ImGui::InputFloat("Jitter", &jitter);

    layerSpacing = std::max(layerSpacing, 0.0f);
    jitter       = std::max(jitter, 0.0f);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    ptableCache.reset();
    return std::make_shared<GraphLayout>(std::move(shell3d_layout_bfs(*graph, *this)));
  }

  std::string getName() override {
    return "Shell3D";
  }
};
struct LinLogCreateInfoGUI : public graphs::position::LinLogCreateInfo, LayoutController {
  void configureUI() override {
    ImGui::SeparatorText("LinLog");

    ImGui::InputFloat("Attraction (k)", &k_attract);
    ImGui::InputFloat("Repulsion (k)", &k_repulse);
    ImGui::InputFloat("Step Size", &step);

    ImGui::InputInt("Max Neighbors", &maxNeighbors);
    ImGui::InputFloat("Max Distance", &maxDistance);

    maxNeighbors = std::max(maxNeighbors, 1);
    maxDistance  = std::max(maxDistance, 0.01f);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    std::shared_ptr<PositionTable> table = ptableCache.getPositionTable(graph, this);
    graphs::position::linlog_step(*table, *graph, *this);
    return table;
  }

  std::string getName() override {
    return "LinLog";
  }
};

struct RadialPlacementCreateInfoGUI : public graphs::position::RadialPlacementCreateInfo, LayoutController {
  void configureUI() override {
    ImGui::SeparatorText("Radial Placement");

    ImGui::SliderFloat("Hub Percentile", &hub_degree_threshold_percentile, 50.f, 99.f);
    ImGui::InputFloat("Peer Attraction", &peer_attraction_strength);
    ImGui::InputFloat("Hub Repulsion", &hub_repulsion_strength);
    ImGui::InputFloat("Hub Spring Length", &hub_spring_length);
    ImGui::InputInt("Hub Iterations", &hub_force_iterations);

    ImGui::InputFloat("Min Distance", &min_distance);
    ImGui::InputFloat("Color Fade Power", &color_fade_power);

    ImGui::Checkbox("Randomize Peer Offset", &randomize_peer_initial_offset);
    ImGui::InputFloat("Peer Cloud Spread", &peer_cloud_spread);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    std::shared_ptr<PositionTable> table = ptableCache.getPositionTable(graph, this);
    return std::make_shared<PositionTable>(graphs::position::radialPlacement(*table, *graph));
  }

  std::string getName() override {
    return "RadialPlacement";
  }
};

struct DegreeGrowthCreateInfoGUI : public graphs::position::DegreeGrowthCreateInfo, LayoutController {

  void configureUI() override {
    ImGui::SeparatorText("Degree Growth");

    ImGui::InputFloat("Base Radius", &base_radius);
    ImGui::InputFloat("Degree Exponent", &degree_exponent);
    ImGui::InputFloat("Angular Jitter", &angular_jitter);

    const char* init_labels[] = {"Origin", "Circle", "From Old"};
    int         init_i        = static_cast<int>(init);

    if (ImGui::Combo("Init Mode", &init_i, init_labels, 3))
      init = static_cast<Init>(init_i);

    if (init == Init::kCircle)
      ImGui::InputFloat("Init Circle Radius", &init_circle_radius);

    ImGui::InputFloat("Min Radius", &min_radius, 0, 0, "%.6f");

    min_radius = std::max(min_radius, 1e-6f);
  }

  std::shared_ptr<PositionTable> layout(std::shared_ptr<Graph> graph) override {
    std::shared_ptr<PositionTable> table = ptableCache.getPositionTable(graph, this);

    PositionTable newtable = graphs::position::degree_growth(*table, *graph, *(DegreeGrowthCreateInfo*)this);

    table->positions.swap(newtable.positions);
    table->colors.swap(newtable.colors);

    return table;
  }

  std::string getName() override {
    return "DegreeGrowth";
  }
};


struct KamadaKawaiCreateInfoGUI : public graphs::position::KamadaKawaiCreateInfo, LayoutController {
  void configureUI() override {
    ImGui::TextUnformatted("Kamada–Kawai Configuration");
    ImGui::Separator();

    ImGui::InputInt("Max Iterations", &max_iterations);
    if (max_iterations < 1) max_iterations = 1;

    ImGui::InputFloat("Epsilon", &epsilon, 0.0f, 0.0f, "%.6f");
    if (epsilon <= 0.0f) epsilon = 1e-6f;

    ImGui::Checkbox("Use Newton Method", &use_newton);

    ImGui::Spacing();
    ImGui::SeparatorText("Energy Model");

    ImGui::InputFloat("L0", &L0);
    ImGui::InputFloat("k Constant Base", &k_constant_base);
    ImGui::InputFloat("Distance Power", &distance_power);

    ImGui::Spacing();
    ImGui::SeparatorText("Initialization");

    const char* init_labels[] = {"Circle", "Random", "From Old"};
    int         init_index    = static_cast<int>(init);
    if (ImGui::Combo("Init Mode", &init_index, init_labels, IM_ARRAYSIZE(init_labels))) {
      init = static_cast<Init>(init_index);
    }

    if (init == Init::kCircle) {
      ImGui::InputFloat("Circle Radius", &init_circle_radius);
    } else if (init == Init::kRandom) {
      ImGui::InputFloat("Random Radius", &init_random_radius);
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Distances & Stability");

    ImGui::InputFloat("Min Distance", &min_distance, 0.0f, 0.0f, "%.8f");
    if (min_distance <= 0.0f) min_distance = 1e-8f;
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    std::shared_ptr<PositionTable> table = ptableCache.getPositionTable(graph, this);

    auto result = graphs::position::kamada_kawai(*table, *graph, *(KamadaKawaiCreateInfo*)this);

    return std::make_shared<PositionTable>(result);
  }

  std::string getName() override {
    return "KamadaKawai";
  }
};

struct ForceDirected3DCreateInfoGUI : LayoutController {
  float speed = 0.1f;

  void configureUI() override {
    ImGui::SeparatorText("Force Directed 3D");
    ImGui::InputFloat("Speed", &speed);
    speed = std::max(speed, 0.0001f);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    auto table = ptableCache.getPositionTable(graph, this);
    graphs::position::force_directed_3d_step(*table, *graph, {}, speed);
    return table;
  }

  std::string getName() override {
    return "Force Directed 3D";
  }
};

struct SpectralLayoutCreateInfoGUI : public graphs::position::SpectralCreateInfo, LayoutController {

  bool use3D = false;

  void configureUI() override {
    ImGui::SeparatorText("Spectral Layout");

    ImGui::InputFloat("Scale", &scale);
    ImGui::Checkbox("Center", &center);
    ImGui::Checkbox("3D", &use3D);

    scale = std::max(scale, 0.0001f);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    if (use3D) {
      auto table = std::make_shared<PositionTable>(
        graphs::position::spectral_layout_3d(*graph, *this));
      return table;
    } else {
      auto table = std::make_shared<PositionTable>(
        graphs::position::spectral_layout_2d(*graph, *this));
      return table;
    }
  }

  std::string getName() override {
    return "Spectral";
  }
};

struct SpringLayoutCreateInfoGUI : public graphs::position::SpringLayoutCreateInfo, LayoutController {

  void configureUI() override {
    ImGui::SeparatorText("Spring Layout");

    ImGui::InputFloat("Rest Length (L0)", &L0);
    ImGui::InputFloat("Spring k", &k_spring);
    ImGui::InputFloat("Repulsion k", &k_repulse);
    ImGui::InputFloat("Speed", &speed);

    L0        = std::max(L0, 0.001f);
    speed     = std::max(speed, 0.0001f);
    k_spring  = std::max(k_spring, 0.00001f);
    k_repulse = std::max(k_repulse, 0.000001f);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    auto table = ptableCache.getPositionTable(graph, this);
    graphs::position::spring_layout_step(*table, *graph, *this);
    return table;
  }

  std::string getName() override {
    return "Spring";
  }
};

struct SpringLayoutOctreeCreateInfoGUI : public graphs::position::SpringLayoutOctreeCreateInfo, LayoutController {

  void configureUI() override {
    ImGui::SeparatorText("Spring Layout (Octree)");

    ImGui::InputFloat("Rest Length (L0)", &L0);
    ImGui::InputFloat("Spring k", &k_spring);
    ImGui::InputFloat("Repulsion k", &k_repulse);
    ImGui::InputFloat("Speed", &speed);

    ImGui::InputInt("Max K Neighbors", &maxK);
    ImGui::InputFloat("Max Distance", &maxDistance);

    maxK        = std::max(maxK, 1);
    maxDistance = std::max(maxDistance, 0.01f);
    speed       = std::max(speed, 0.0001f);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    auto table = ptableCache.getPositionTable(graph, this);
    graphs::position::spring_layout_step_octree(*table, *graph, *this);
    return table;
  }

  std::string getName() override {
    return "Spring (Octree)";
  }
};

struct StressMajorizationCreateInfoGUI : public graphs::position::StressMajorizationCreateInfo, LayoutController {

  void configureUI() override {
    ImGui::SeparatorText("Stress Majorization");

    ImGui::InputFloat("Base Length (L0)", &L0);
    ImGui::InputFloat("Weight Power", &weight_power);
    ImGui::InputInt("Max Iterations", &max_iterations);
    ImGui::InputFloat("Epsilon", &epsilon);

    const char* init_modes[] = {"Circle", "Random", "From Old"};
    int         init_i       = static_cast<int>(init);
    if (ImGui::Combo("Initialization", &init_i, init_modes, 3))
      init = static_cast<Init>(init_i);

    ImGui::InputFloat("Circle Radius", &init_circle_radius);
    ImGui::InputFloat("Random Radius", &init_random_radius);
    ImGui::Checkbox("Use APSP Cache", &use_apsp_cache);

    max_iterations = std::max(max_iterations, 1);
    epsilon        = std::max(epsilon, 1e-8f);
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    auto table = ptableCache.getPositionTable(graph, this);
    graphs::position::stress_majorization_step(*table, *graph, *this);
    return table;
  }

  std::string getName() override {
    return "Stress Majorization";
  }
};

struct ACILayoutGUI : public graphs::position::AlgorithmACI, public LayoutController {
  graphs::position::AlgorithmACI newCi;

  bool _shouldLayout = false;
  bool _huboptimize  = false;

  int   iterationCount         = 10;
  float forceMultiplier        = 20.0f;
  float delta                  = 0.1f;
  float hubThreshold           = 0.11f;
  float treecapitationBase     = 4.0f;
  float treecapitationExponent = 0.0f;
  int   selection              = tween::CUBICIN;

  bool showDebug = false;

  void configureUI() override {
    ImGui::SeparatorText("ACI Layout");

    ImGui::SliderFloat("Major Distance", &newCi.majorDistance, 0.0f, 200.0f);
    ImGui::SliderFloat("Minor Distance", &newCi.minorDistance, 0.0f, 100.0f);
    ImGui::SliderFloat("TreeCapitation", &treecapitationBase, 0.0f, 50.0f);
    ImGui::InputFloat("TreeCapitation (Exp)", &treecapitationExponent, 0.0f, 50.0f);
    ImGui::SliderFloat("Log Tolerance", &newCi.logtolerance, 0.0f, 1.0f);
    ImGui::SliderFloat("Minimal", &newCi.minimal, 0.0f, 1.0f);
    ImGui::InputFloat("Offset", &newCi.offset);
    ImGui::SliderFloat("Threshold", &newCi.nodeThreshold, 0.0f, 1.0f);

    int modified = ImGui::Curve("Curve", ImVec2(400, 200), 3, (ImVec2*)newCi.interp, &selection);
    ImGui::Checkbox("Interpolation", &newCi.interpolation);

    newCi.treecapitation = treecapitationBase * std::pow(10.0f, treecapitationExponent);

    ImGui::SliderFloat("C", &newCi.cv, 0.0f, 8.0f);
    ImGui::SliderFloat("B", &newCi.bv, 0.0f, 8.0f);
    ImGui::SliderFloat("A", &newCi.av, 0.0f, 8.0f);
    ImGui::Separator();


    if (!(newCi == *this) || modified) {
      static_cast<graphs::position::AlgorithmACI&>(*this) = newCi;
      _shouldLayout                                       = true;
    }

    ImGui::Text("Hub optimize");
    ImGui::InputFloat("Force multiplier", &forceMultiplier);
    ImGui::InputFloat("Delta", &delta);
    ImGui::InputInt("IterationCount", &iterationCount);
    ImGui::SliderFloat("HubThreshold", &hubThreshold, 0.0f, 1.0f);

    if (!_huboptimize)
      _huboptimize = ImGui::Button("Start");

    if (_huboptimize) {
      _huboptimize = !ImGui::Button("Stop");
      treecapitatorReset();
    }

    majorDistance = std::max(majorDistance, 0.0f);

    ImGui::Checkbox("ShowDebug", &showDebug);
    if (showDebug) {
      ImGui::Separator();
      ImGui::Text("%f %f", interp[0].x, interp[0].y);
      ImGui::Text("%f %f", interp[1].x, interp[1].y);
      ImGui::Text("%f %f", interp[2].x, interp[2].y);
    }
  }

  std::shared_ptr<GraphLayout> layout(std::shared_ptr<Graph> graph) override {
    ptableCache.reset();

    auto graphlayout = ptableCache.getPositionTable(graph, this);
    graphs::position::treecapitator(*graph, graphlayout, static_cast<const graphs::position::AlgorithmACI&>(*this));
    treecapitatorReset();

    return graphlayout;
  }

  void optimize(std::shared_ptr<Graph> graph, std::shared_ptr<GraphLayout> layout) override {
    graphs::position::treecapitatorForceDirected(graph, static_cast<const graphs::position::AlgorithmACI&>(*this), layout, iterationCount, delta, forceMultiplier, hubThreshold);
  }

  bool shouldLayout() override {
    if (_shouldLayout) {
      _shouldLayout = false;
      return true;
    }
    return false;
  }

  bool shouldOptimize() override {
    return _huboptimize;
  }

  std::string getName() override {
    return "TreeCapitator";
  }
};

std::vector<LayoutController*> getLayouts() {
  static std::vector<LayoutController*> layouts;

  if (layouts.empty()) {
    layouts.push_back(new KamadaKawaiCreateInfoGUI);
    layouts.push_back(new Shell3DCreateInfoGUI);
    layouts.push_back(new LinLogCreateInfoGUI);
    layouts.push_back(new DegreeGrowthCreateInfoGUI);
    layouts.push_back(new ForceDirected3DCreateInfoGUI);
    layouts.push_back(new SpringLayoutCreateInfoGUI);
    layouts.push_back(new SpringLayoutOctreeCreateInfoGUI);
    layouts.push_back(new SpectralLayoutCreateInfoGUI);
    layouts.push_back(new StressMajorizationCreateInfoGUI);
    layouts.push_back(new RadialPlacementCreateInfoGUI);
    layouts.push_back(new ACILayoutGUI);
    layouts.push_back(new sunflowerGUI);
  }

  return layouts;
}

} // namespace application
