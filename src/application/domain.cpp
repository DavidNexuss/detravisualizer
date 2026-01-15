#include "domain.hpp"
#include "util.hpp"
#include <filesystem>
#include <fstream>
#include <zlib.h>

static const std::string GRAPH_DIR = "networks/";

namespace domain {

std::vector<std::string> graphlist() {

  std::vector<std::string> graphs;

  if (!std::filesystem::exists(GRAPH_DIR))
    return graphs;

  for (const auto& entry : std::filesystem::directory_iterator(GRAPH_DIR)) {

    if (!entry.is_regular_file())
      continue;

    const auto filename = entry.path().filename().string();

    if (filename.size() >= 4 &&
        (filename.ends_with(".txt") ||
         filename.ends_with(".txt.gz") ||
         filename.ends_with(".conllu"))) {

      graphs.push_back(filename);
    }
  }

  return graphs;
}
std::shared_ptr<Conllu> graphloadConllu(const std::string& filepath) {

  const std::string path = "networks/" + filepath;

  std::ifstream file(path);
  if (!file.is_open()) {
    return nullptr;
  }

  auto result    = std::make_shared<Conllu>();
  result->graph  = std::make_shared<Graph>();
  result->labels = std::make_shared<std::unordered_map<uint32_t, std::vector<std::string>>>();

  std::string line;

  uint32_t nodeOffset    = 0;
  uint32_t sentenceMaxId = 0;

  uint32_t vertexCount = 0;

  while (std::getline(file, line)) {

    if (line.empty()) {
      nodeOffset += sentenceMaxId;
      sentenceMaxId = 0;
      continue;
    }

    if (line[0] == '#')
      continue;


    std::string token;

    int u = -1;
    int v = -1;

    std::stringstream ss(line);

    while (ss >> token) {
      try {
        u = std::stoi(token);
        break;
      } catch (...) {}
    }

    while (ss >> token) {
      try {
        v = std::stoi(token);
        break;
      } catch (...) {}
    }

    if (u != -1 && v != -1) {

      int U = u + nodeOffset;
      int V = v + nodeOffset;

      int K = std::max(U, V);
      if (K >= vertexCount) {
        result->graph->addVertices(K - vertexCount + 1);
        vertexCount = K + 1;
      }

      if (U >= result->graph->getVertexCount() || V >= result->graph->getVertexCount()) {
        std::cout << "CRASH" << std::endl;
        std::cout << result->graph->getVertexCount() << " " << U << " " << V << std::endl;
      }
      result->graph->addEdge(U, V);
      sentenceMaxId++;
    }
  }

  return result;
}

std::shared_ptr<Graph> graphloadTxt(const std::string& filepath) {
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

std::shared_ptr<Graph> graphloadGzip(const std::string& filepath) {

  const std::string path = "networks/" + filepath;

  gzFile file = gzopen(path.c_str(), "rb");
  if (!file) {
    throw std::runtime_error("Failed to open .gz file");
  }

  auto graph = std::make_shared<Graph>();

  constexpr size_t BufferSize = 4096;

  char buffer[BufferSize];

  std::string pending;

  int bytesRead = 0;

  while ((bytesRead = gzread(file, buffer, BufferSize)) > 0) {

    pending.append(buffer, bytesRead);

    size_t pos = 0;
    while ((pos = pending.find('\n')) != std::string::npos) {

      std::string line = pending.substr(0, pos);
      pending.erase(0, pos + 1);

      if (line.empty() || line[0] == '#')
        continue;

      int                u, v;
      std::istringstream iss(line);
      if (iss >> u >> v) {
        graph->addEdge(u, v);
      }
    }
  }

  gzclose(file);
  return graph;
}


std::shared_ptr<Graph> graphload(const std::string& filepath) {
  std::string ext = std::filesystem::path(filepath).extension().string();

  if (ext == ".gz") return graphloadGzip(filepath);
  if (ext == ".txt") return graphloadTxt(filepath);
  if (ext == ".conllu") return graphloadConllu(filepath)->graph;

  return {};
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
