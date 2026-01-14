#include <position.hpp>
#include <detrarandom/random_sources.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <metrics.hpp>
#include <unordered_set>
#include <application/util.hpp>
#include <memory>

namespace graphs {
namespace position {
struct AlgorithmACI {
  float majorDistance = 100.0f;
};

static random_sources::XORand source;

template <typename Graph>
void aci(Graph& graph, std::shared_ptr<GraphLayout> layout, AlgorithmACI ci) {
  if (graph.getVertexCount() == 0) return;

  // Gets degree of each node
  auto degrees = metrics::degree_sequence(graph);

  std::vector<uint32_t> degreeSequence(graph.getVertexCount());

  std::iota(degreeSequence.begin(), degreeSequence.end(), 0);

  // Sort node indexes by degree decreasingly, higher degrees first
  std::sort(degreeSequence.begin(), degreeSequence.end(), [&](auto& lhs, auto& rhs) {
    return degrees[lhs] > degrees[rhs];
  });

  // log of greatest degree
  float maxDegree = std::log(graph.getEdgeCount(degreeSequence[0]) + 1);

  std::vector<glm::vec3> positions(graph.getVertexCount());

  std::unordered_set<uint32_t> placed;

  //Calculate postions A and B
  for (uint32_t node : degreeSequence) {

    glm::vec3 A = glm::vec3(0.0f);
    glm::vec3 B = glm::vec3(0.0f);

    //Computation of A
    //How relevant is this node as peer, based on how meaninful are its connection to its peers
    {
      std::vector<uint32_t>  relevant;
      std::vector<glm::vec3> relevantPositions;

      // Only look for nodes that already have a position in the layout, which are guranteed to have higher degree
      for (uint32_t c : graph.getEdges(node)) {
        if (placed.count(c)) {
          relevant.push_back(c);
          relevantPositions.push_back(positions[c]);
        }
      }

      float totalNeighbourWeight = 0.0f;

      for (uint32_t u : relevant) {
        totalNeighbourWeight += std::log(graph.getEdgeCount(u) + 1.0f);
      }

      float totalWeight = 0.0f;
      //Maximize centroid position around most meaningful nodes, probably this will need a scaling over the actual weight
      for (int i = 0; i < relevant.size(); i++) {
        float w = 1.0 - (std::log(graph.getEdgeCount(relevant[i]) + 1.0f) / totalNeighbourWeight);
        totalWeight += w;
        A += w * relevantPositions[i];
      }

      if (totalWeight > 0.0f)
        A /= totalWeight;
    }

    //Copmutation of B
    {
      glm::vec3 dir;
      do {
        dir = glm::vec3(
          source.randf() * 2.0f - 1.0f,
          source.randf() * 2.0f - 1.0f,
          source.randf() * 2.0f - 1.0f);
      } while (glm::dot(dir, dir) < 1e-4f);

      dir = glm::normalize(dir);

      float r = ci.majorDistance;

      B = dir * r;
    }

    // Decide how meaniful is this node as hub or peer based on its node degree compared against the maximal node
    float t = std::log(graph.getEdgeCount(node) + 1) / maxDegree;

    // Place the node, linear interpolation of A and B
    positions[node] = t * B + (1 - t) * A;
    placed.insert(node);
  }

  layout->positions = positions;
}

} // namespace position
} // namespace graphs
