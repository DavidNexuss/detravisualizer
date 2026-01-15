#include <position.hpp>
#include <detrarandom/random_sources.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <metrics.hpp>
#include <unordered_set>
#include <memory>
#include <application/domain.hpp>
#include <application/util.hpp>
#include <string.h>
#include <glm/glm.hpp>

static inline glm::vec2 catmullRom(
  const glm::vec2& p0,
  const glm::vec2& p1,
  const glm::vec2& p2,
  const glm::vec2& p3,
  float            t) {
  float t2 = t * t;
  float t3 = t2 * t;

  return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}


float eval(glm::vec2 points[3], float t) {
  t = glm::clamp(t, 0.0f, 1.0f);

  glm::vec2 p0 = points[0];
  glm::vec2 p1 = points[0];
  glm::vec2 p2 = points[1];
  glm::vec2 p3 = points[2];

  glm::vec2 v = catmullRom(p0, p1, p2, p3, t);
  return v.y; // or return v for full 2D
}


namespace graphs {
namespace position {

struct AlgorithmACI {
  float majorDistance  = 100.0f;
  float minorDistance  = 2.0f;
  float treecapitation = 5.0f;
  float logtolerance   = 0.0f;
  float jitter         = 1.0f;
  float minimal        = 0.0f;
  float offset         = 0.0f;
  bool  interpolation  = false;
  float av             = 1.0f;
  float bv             = 1.0f;
  float cv             = 1.0f;
  float nodeThreshold  = 1.0f;

  glm::vec2 interp[3];

  inline bool operator==(const AlgorithmACI& other) const {
    return memcmp(this, &other, sizeof(AlgorithmACI)) == 0;
  }
};

static random_sources::XORand source;

glm::vec3 randomMaxSatValueColor() {
  float h = source.randf();
  float s = 1.0f;
  float v = 1.0f;

  float c = v * s;
  float x = c * (1.0f - std::fabs(std::fmod(h * 6.0f, 2.0f) - 1.0f));
  float m = v - c;

  int sector = static_cast<int>(h * 6.0f);

  glm::vec3 rgb;
  switch (sector) {
    case 0: rgb = {c, x, 0}; break;
    case 1: rgb = {x, c, 0}; break;
    case 2: rgb = {0, c, x}; break;
    case 3: rgb = {0, x, c}; break;
    case 4: rgb = {x, 0, c}; break;
    default: rgb = {c, 0, x}; break;
  }

  return rgb + glm::vec3(m);
}

inline glm::vec3 randomDirection() {

  glm::vec3 dir;
  do {
    dir = glm::vec3(
      source.randf() * 2.0f - 1.0f,
      source.randf() * 2.0f - 1.0f,
      source.randf() * 2.0f - 1.0f);
  } while (glm::dot(dir, dir) < 1e-4f);

  dir = glm::normalize(dir);
  return dir;
}


template <typename T>
std::vector<float> softmin(const std::vector<T>& x, double tau) {
  std::vector<float> weights(x.size());

  if (x.empty())
    return weights;

  T min_x = *std::min_element(x.begin(), x.end());

  float sum = 0.0;
  for (size_t i = 0; i < x.size(); ++i) {
    weights[i] = std::exp(-((float)(x[i] - min_x)) / tau);
    sum += weights[i];
  }

  for (float& w : weights)
    w /= sum;

  return weights;
}
template <typename T>
std::vector<float> softmax(const std::vector<T>& x, double tau) {
  std::vector<T> neg(x.size());
  for (size_t i = 0; i < x.size(); ++i)
    neg[i] = -x[i];

  return softmin(neg, tau);
}


template <typename Graph>
void treecapitatorstep(Graph& graph, AlgorithmACI ci, std::vector<glm::vec3>& colors, std::vector<glm::vec3>& positions, std::unordered_set<uint32_t> ignore, std::vector<uint32_t>& degreeSequence, std::vector<uint32_t>& degrees) {

  source = random_sources::XORand();

  float maxDegree    = graph.getEdgeCount(degreeSequence[0]);
  float maxDegreeLog = std::log(maxDegree + 1);

  std::unordered_set<uint32_t> placed;

  int i = 0;

  bool inverse = false;

  if (inverse)
    std::reverse(std::begin(degreeSequence), std::end(degreeSequence));
  //Calculate postions A and B
  for (uint32_t node : degreeSequence) {
    i++;
    if (ignore.count(node)) {
      placed.insert(node);
      continue;
    }

    if ((i / (float)degreeSequence.size()) > ci.nodeThreshold) {
      positions[node] = glm::vec3(0.0f);
      placed.insert(node);
      continue;
    }

    if (degrees[node] == 0) {
      positions[node] = glm::vec3(0.0f);
      placed.insert(node);
      continue;
    }

    glm::vec3 A  = glm::vec3(0.0f);
    glm::vec3 Ac = glm::vec3(0.0f);
    glm::vec3 B  = glm::vec3(0.0f);
    glm::vec3 C  = glm::vec3(0.0f);

    //Computation of A
    //How relevant is this node as peer, based on how meaninful are its connection to its peers
    {
      std::vector<glm::vec3> relevantPositions;
      std::vector<glm::vec3> relevantColors;
      std::vector<uint32_t>  relevantDegrees;

      // Only look for nodes that already have a position in the layout, which are guranteed to have higher degree
      for (uint32_t c : graph.getEdges(node)) {
        if (placed.count(c)) {
          relevantPositions.push_back(positions[c]);
          relevantColors.push_back(colors[c]);
          relevantDegrees.push_back(graph.getEdgeCount(c));
        }
      }

      auto weights = !inverse ? softmin(relevantDegrees, ci.treecapitation) : softmax(relevantDegrees, ci.treecapitation);

      for (uint32_t i = 0; i < weights.size(); i++) {
        A += weights[i] * relevantPositions[i];
        Ac += weights[i] * relevantColors[i];
      }
    }

    //Computation of B
    {
      B = randomDirection() * ci.majorDistance;
    }

    //Computation of C
    {
      C = randomDirection() * ci.minorDistance;
    }

    // Decide how meaniful is this node as hub or peer based on its node degree compared against the maximal node
    float tnor = graph.getEdgeCount(node) / maxDegree;
    float tlog = std::log(graph.getEdgeCount(node) + 0.0001f) / maxDegreeLog;

    float t = ci.logtolerance * tlog + (1 - ci.logtolerance) * tnor;

    if (ci.interpolation)
      t = eval(ci.interp, t);

    if (inverse)
      t = 1 - t;

    // Place the node, linear interpolation of A and B
    positions[node] =
      ((t + ci.minimal) * B) * ci.bv +
      (1 - t) * A * ci.av +
      C * ci.cv;

    colors[node] = t * randomMaxSatValueColor() + (1 - t) * Ac;

    placed.insert(node);
  }
}

template <typename Graph>
void treecapitator(Graph& graph, std::shared_ptr<GraphLayout> layout, AlgorithmACI ci) {
  if (graph.getVertexCount() == 0) return;

  // Gets degree of each node
  auto degrees = metrics::degree_sequence(graph);

  std::vector<uint32_t> degreeSequence(graph.getVertexCount());

  std::iota(degreeSequence.begin(), degreeSequence.end(), 0);

  // Sort node indexes by degree decreasingly, higher degrees first
  std::sort(degreeSequence.begin(), degreeSequence.end(), [&](auto& lhs, auto& rhs) {
    return degrees[lhs] > degrees[rhs];
  });

  std::vector<glm::vec3> positions(graph.getVertexCount());
  layout->colors = positions;

  treecapitatorstep(graph, ci, layout->colors, positions, {}, degreeSequence, degrees);

  layout->positions = positions;
}



struct ComputeContext {
  std::shared_ptr<graphs::Graph<graphs::backends::AdjacencyListVector>> lastGraph;

  std::vector<glm::vec3> velocity;
  std::vector<float>     lengths;
  std::vector<float>     weights;
  std::vector<uint32_t>  hubs;
  std::vector<uint32_t>  degrees;
  std::vector<uint32_t>  degreeSequence;
};

static ComputeContext ctx = {};


void treecapitatorReset() {
  ctx = {};
}
template <typename Graph>
void treecapitatorForceDirected(std::shared_ptr<Graph> graph, AlgorithmACI ci, std::shared_ptr<GraphLayout> layout, int iterations, float delta, float k, float HubThreshold) {
  if (layout == nullptr) return;
  if (graph == nullptr) return;

  if (ctx.lastGraph != graph) {
    ctx           = {};
    ctx.lastGraph = graph;

    // Yes I know, inconsitent naming
    ctx.degrees = metrics::degree_sequence(*graph);

    ctx.degreeSequence = std::vector<uint32_t>(graph->getVertexCount());

    //Typing laziness
    auto& indices = ctx.degreeSequence;

    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](const auto& lhs, const auto& rhs) {
      return ctx.degrees[lhs] > ctx.degrees[rhs];
    });

    int topK = 0;

    for (int i = 0; i < indices.size(); i++) {
      if (ctx.degrees[indices[i]] >= (ctx.degrees[indices[0]] * HubThreshold)) topK++;
      else
        break;
    }

    ctx.velocity = std::vector<glm::vec3>(topK);
    ctx.lengths  = std::vector<float>(topK);
    ctx.weights  = std::vector<float>(topK);

    ctx.hubs = indices;
    ctx.hubs.resize(topK);

    for (uint32_t i = 0; i < topK; i++) {
      ctx.lengths[i] = glm::length(layout->positions[indices[i]]);
      ctx.weights[i] = graph->getEdgeCount(indices[i]) / (float)graph->getEdgeCount(indices[0]);
    }
  }

  // We want to keep the vector magnitudes
  std::vector<glm::vec3>& positions = layout->positions;

  for (int i = 0; i < iterations; i++) {
    for (uint32_t u = 0; u < ctx.hubs.size(); u++) {
      uint32_t  uindex = ctx.hubs[u];
      glm::vec3 force  = glm::vec3(0.0f);

      for (uint32_t v = 0; v < ctx.hubs.size(); v++) {
        if (u == v) continue;

        uint32_t  vindex = ctx.hubs[v];
        glm::vec3 d      = positions[uindex] - positions[vindex];

        float len = glm::length(d);

        if (len < 1e-4f) { len = 1e-4f; }

        float w = k * ctx.weights[u] * ctx.weights[v] / (len * len);

        force += (d * (1.0f / len) * w);
      }

      ctx.velocity[u] += force * delta;
    }

    for (uint32_t u = 0; u < ctx.hubs.size(); u++) {
      positions[u] += ctx.velocity[u] * delta;
      positions[u] = glm::normalize(positions[u]);
      positions[u] = positions[u] * ctx.lengths[u];
    }
  }

  std::unordered_set<uint32_t> ignore;

  for (uint32_t u = 0; u < ctx.hubs.size(); u++) ignore.insert(ctx.hubs[u]);

  treecapitatorstep(*graph, ci, layout->colors, positions, ignore, ctx.degreeSequence, ctx.degrees);
}
} // namespace position
} // namespace graphs
