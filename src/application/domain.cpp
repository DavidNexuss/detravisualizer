#include "domain.hpp"
#include "util.hpp"
#include <filesystem>
#include <fstream>

static const std::string GRAPH_DIR = "networks/";

namespace domain {

std::vector<std::string> graphlist() {
  std::vector<std::string> graphs;

  if (!std::filesystem::exists(GRAPH_DIR)) {
    return graphs;
  }

  for (const auto& entry : std::filesystem::directory_iterator(GRAPH_DIR)) {
    if (entry.is_regular_file()) {
      graphs.push_back(entry.path().filename().string());
    }
  }

  return graphs;
}

std::shared_ptr<Graph> graphload(const std::string& filepath) {

  const std::string& path = "networks/" + filepath;

  std::ifstream file(path);
  if (!file.is_open()) {
    return nullptr;
  }

  std::shared_ptr<Graph> graph = std::make_shared<Graph>();

  std::string line;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;

    std::istringstream iss(line);

    int u, v;
    if (iss >> u >> v) {
      graph->addEdge(u, v);
    }
  }

  return graph;
}

bool graphsave(std::shared_ptr<Graph> graph, const std::string& filepath, bool overwrite) {

  if (!graph) {
    return false;
  }

  const std::string& path = "networks/" + filepath;

  if (std::filesystem::exists(path) && !overwrite) {
    return false;
  }

  std::ofstream file(path, std::ios::trunc);
  if (!file.is_open()) {
    return false;
  }

  for (uint64_t i = 0; i < graph->getVertexCount(); i++) {
    auto edges = graph->getEdges(i);
    if (edges.size()) {
      for (size_t v : edges) {
        file << i << " " << v << "\n";
      }
    }
  }

  return true;
}

} // namespace domain
