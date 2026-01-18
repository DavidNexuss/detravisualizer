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
  std::ifstream     file(path);
  if (!file.is_open()) {
    return nullptr;
  }

  auto result    = std::make_shared<Conllu>();
  result->graph  = std::make_shared<Graph>();
  result->labels = std::make_shared<std::unordered_map<uint32_t, std::vector<std::string>>>();

  // Global aggregation
  std::unordered_map<std::string, uint32_t> lemmaToNode;

  // Sentence-local
  std::unordered_map<int, std::string> tokenLemma;
  std::unordered_map<int, int>         tokenHead;

  std::string line;

  auto getNode = [&](const std::string& lemma) -> uint32_t {
    auto it = lemmaToNode.find(lemma);
    if (it != lemmaToNode.end())
      return it->second;

    uint32_t id = result->graph->getVertexCount();
    result->graph->addVertices(1);
    lemmaToNode[lemma] = id;
    (*result->labels)[id].push_back(lemma);
    return id;
  };

  auto flushSentence = [&]() {
    for (auto& p : tokenHead) {
      int dep  = p.first;
      int head = p.second;

      if (head == 0) continue; // root

      uint32_t u = getNode(tokenLemma[head]);
      uint32_t v = getNode(tokenLemma[dep]);

      result->graph->addEdge(u, v);
    }
    tokenLemma.clear();
    tokenHead.clear();
  };

  while (std::getline(file, line)) {

    if (line.empty()) {
      flushSentence();
      continue;
    }

    if (line[0] == '#')
      continue;

    // TAB-based split (this is the fix)
    std::vector<std::string> cols;
    std::string              field;
    std::stringstream        ss(line);
    while (std::getline(ss, field, '\t'))
      cols.push_back(field);

    if (cols.size() < 8)
      continue;

    // Skip multi-word tokens (e.g. 38-39)
    if (cols[0].find('-') != std::string::npos)
      continue;

    int id, head;
    try {
      id   = std::stoi(cols[0]);
      head = std::stoi(cols[6]);
    } catch (...) {
      continue; // defensive
    }

    const std::string& lemma = cols[2];

    tokenLemma[id] = lemma;
    tokenHead[id]  = head;
  }

  flushSentence();
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
