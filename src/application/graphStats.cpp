#include <stdlib.h>
#include "graphStats.hpp"
#include <metrics.hpp>
#include <glm/ext.hpp>
#include <imgui/imgui.h>
#include <implot/implot.h>

struct FitResult {
  double gamma = 0.0;
  double r2    = 0.0;
  double slope = 0.0;
};

FitResult fit(const std::vector<float>& logX, const std::vector<float>& logY, size_t start = 0, size_t end = std::numeric_limits<size_t>::max()) {
  FitResult fit;

  if (logX.size() != logY.size() || logX.empty())
    return fit;

  if (end > logX.size())
    end = logX.size();

  if (start + 2 >= end)
    return fit;

  double sumX = 0.0, sumY = 0.0;
  double sumXX = 0.0, sumXY = 0.0;

  const size_t N = end - start;

  for (size_t i = start; i < end; ++i) {
    const double x = logX[i];
    const double y = logY[i];

    sumX += x;
    sumY += y;
    sumXX += x * x;
    sumXY += x * y;
  }

  const double denom = N * sumXX - sumX * sumX;
  if (denom == 0.0)
    return fit;

  const double slope     = (N * sumXY - sumX * sumY) / denom;
  const double intercept = (sumY - slope * sumX) / double(N);

  double ssTot = 0.0;
  double ssRes = 0.0;

  const double meanY = sumY / double(N);

  for (size_t i = start; i < end; ++i) {
    const double y    = logY[i];
    const double yFit = slope * logX[i] + intercept;

    ssTot += (y - meanY) * (y - meanY);
    ssRes += (y - yFit) * (y - yFit);
  }

  fit.r2    = (ssTot > 0.0) ? (1.0 - ssRes / ssTot) : 0.0;
  fit.slope = slope;

  fit.gamma = 1.0 + 1.0 / std::abs(slope);

  return fit;
}

void GraphStatistics::reload(std::shared_ptr<Graph> graph) {
  if (graph == nullptr) return;

  nodeCount = graph->getVertexCount();
  edgeCount = graph->getEdgeCount();

  nodeDegree = nodeDegreeSorted = graphs::metrics::degree_sequence(*graph);

  std::sort(nodeDegreeSorted.begin(), nodeDegreeSorted.end(), std::greater<>());

  nodeDegreeIota.resize(nodeDegree.size());
  nodeDegreeSortedLog.resize(nodeDegree.size());
  nodeDegreeIotaLog.resize(nodeDegree.size());
  ImGui::Text("Node count layout: %lu", nodeCountLayout);

  std::iota(nodeDegreeIota.begin(), nodeDegreeIota.end(), 1);

  for (size_t i = 0; i < nodeDegreeSorted.size(); i++) {
    nodeDegreeSortedLog[i] = std::log(nodeDegreeSorted[i]);
    nodeDegreeIotaLog[i]   = std::log(nodeDegreeIota[i]);
  }

  auto result = fit(nodeDegreeIotaLog, nodeDegreeSortedLog);

  this->fittedGamma = result.gamma;
  this->fittedSlope = result.slope;
  this->fittedr2    = result.r2;
}

void GraphStatistics::ui() {
  ImGui::Text("Graph stats popup");
  if (nodeCount == 0) return;

  ImGui::Text("Node count: %lu", nodeCount);
  ImGui::Text("Node edge count: %lu", edgeCount);
  ImGui::Text("Gamma: %f", this->fittedGamma);
  ImGui::Text("Slope: %f", this->fittedSlope);
  ImGui::Text("R2: %f", this->fittedr2);
  ImGui::Separator();

  static bool showLowerboundNodeDegree = false;

  ImGui::Checkbox("Show expected degree curve", &showLowerboundNodeDegree);

  if (ImPlot::BeginPlot("DegreeSequence")) {
    ImPlot::SetupAxes("rank", "deg");
    ImPlot::PlotLine("DegreeSequence", nodeDegreeIota.data(), nodeDegreeSorted.data(), nodeDegreeSorted.size());
    ImPlot::EndPlot();
  }

  if (ImPlot::BeginPlot("DegreeSequence LogLog")) {
    /* 
    std::vector<float> nodedegreeTheoretical(nodeDegreeIota.size());

    for (int i = 0; i < nodedegreeTheoretical.size(); i++) {
      nodedegreeTheoretical[i] = nodeDegreeIota.size() * std::pow(i, -1.0f) + std::log(i);
    } */


    ImPlot::SetupAxes("log(rank)", "log(deg)");
    ImPlot::PlotLine("DegreeSequence", nodeDegreeIotaLog.data(), nodeDegreeSortedLog.data(), nodeDegreeSortedLog.size());
    //ImPlot::PlotLine("Theoretical", nodeDegreeIotaLog.data(), nodedegreeTheoretical.data(), nodeDegreeSortedLog.size());
    ImPlot::EndPlot();
  }
}


void GraphLayoutStatistics::ui() {
  shouldComputeBoxCounting = ImGui::Button("BoxCount");
  if (boxcountingResultCounter.size() && ImPlot::BeginPlot("Fractal dimension")) {
    ImPlot::SetupAxes("log(scale)", "log(counting)");
    ImPlot::PlotLine("Ratio", boxcountingResultScale.data(), boxcountingResultCounter.data(), boxcountingResultCounter.size());
    ImPlot::EndPlot();
  }
}

struct ivec3_hash {
  std::size_t operator()(const glm::ivec3& v) const noexcept {
    std::size_t h1 = std::hash<int>{}(v.x);
    std::size_t h2 = std::hash<int>{}(v.y);
    std::size_t h3 = std::hash<int>{}(v.z);

    std::size_t seed = h1;
    seed ^= h2 + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    seed ^= h3 + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);

    return seed;
  }
};

struct BoxCountingDS {
  std::unordered_set<glm::ivec3, ivec3_hash> counter;
  float                                      voxelSize;

  void raster(glm::vec3 center, float radius) {
    glm::ivec3 minVoxel(
      int(std::floor((center.x - radius) / voxelSize)),
      int(std::floor((center.y - radius) / voxelSize)),
      int(std::floor((center.z - radius) / voxelSize)));

    glm::ivec3 maxVoxel(
      int(std::floor((center.x + radius) / voxelSize)),
      int(std::floor((center.y + radius) / voxelSize)),
      int(std::floor((center.z + radius) / voxelSize)));

    const float r2 = radius * radius;

    for (int x = minVoxel.x; x <= maxVoxel.x; ++x) {
      for (int y = minVoxel.y; y <= maxVoxel.y; ++y) {
        for (int z = minVoxel.z; z <= maxVoxel.z; ++z) {

          glm::vec3 boxMin(
            x * voxelSize,
            y * voxelSize,
            z * voxelSize);

          glm::vec3 boxMax = boxMin + glm::vec3(voxelSize);

          glm::vec3 closest(
            std::max(boxMin.x, std::min(center.x, boxMax.x)),
            std::max(boxMin.y, std::min(center.y, boxMax.y)),
            std::max(boxMin.z, std::min(center.z, boxMax.z)));

          glm::vec3 d = closest - center;

          if (glm::dot(d, d) <= r2) {
            counter.insert({x, y, z});
          }
        }
      }
    }
  }

  float boxhitCounter() { return counter.size(); }
};

void GraphLayoutStatistics::computeBoxCounting(std::shared_ptr<Graph> graph, std::shared_ptr<GraphLayout> layout) {
  if (graph.get() == nullptr || layout.get() == nullptr) return;

  boxcountingResultScale.clear();
  boxcountingResultCounter.clear();

  float currentScale = boxCountingVoxelSize;

  std::cout << "voxel size " << boxCountingVoxelSize << std::endl;
  for (int i = 0; i < boxCountingIterations; i++) {
    BoxCountingDS ds;
    ds.voxelSize = currentScale;
    std::cout << currentScale << std::endl;
    for (uint32_t u = 0; u < graph->getVertexCount(); u++) {
      ds.raster(layout->positions[u], boxCountingPointRadius);
    }

    currentScale *= boxCountingScaling;
    boxcountingResultCounter.push_back(std::log(1.0f / currentScale));
    boxcountingResultScale.push_back(std::log(ds.boxhitCounter()));

    std::cout << "Ieration result:  " << i << " " << currentScale << " " << ds.boxhitCounter() << std::endl;
    std::cout << "Ieration result:  log " << i << " scale " << std::log(1.0f / currentScale) << " hit " << std::log(ds.boxhitCounter()) << std::endl;
  }
}

void GraphLayoutStatistics::reload(std::shared_ptr<Graph> graph, std::shared_ptr<GraphLayout> layout) {
  glm::vec3 centroid = glm::vec3(0.0);

  for (int i = 0; i < layout->positions.size(); i++) {
    centroid += layout->positions[i];
  }

  centroid *= 1.0f / (float)layout->positions.size();

  centroid  = centroid;
  transform = glm::translate(glm::mat4(1.0f), -centroid);

  boxcountingResultCounter.clear();
  boxcountingResultScale.clear();
}
