#include <stdlib.h>
#include "graphStats.hpp"
#include <metrics.hpp>
#include <glm/ext.hpp>
#include <imgui/imgui.h>
#include <implot/implot.h>

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
}

void GraphStatistics::ui() {
  ImGui::Text("Graph stats popup");
  if (nodeCount == 0) return;

  ImGui::Text("Node count: %lu", nodeCount);
  ImGui::Text("Node edge count: %lu", edgeCount);
  ImGui::Separator();

  if (ImPlot::BeginPlot("DegreeSequence")) {
    ImPlot::SetupAxes("rank", "deg");
    ImPlot::PlotLine("DegreeSequence", nodeDegreeIota.data(), nodeDegreeSorted.data(), nodeDegreeSorted.size());
    ImPlot::EndPlot();
  }

  if (ImPlot::BeginPlot("DegreeSequence LogLog")) {
    ImPlot::SetupAxes("log(rank)", "log(deg)");
    ImPlot::PlotLine("DegreeSequence", nodeDegreeIotaLog.data(), nodeDegreeSortedLog.data(), nodeDegreeSortedLog.size());
    ImPlot::EndPlot();
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
}
