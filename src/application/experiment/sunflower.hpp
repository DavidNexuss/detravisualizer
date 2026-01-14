#include <glm/glm.hpp>
#include <detrarandom/random_sources.hpp>


static random_sources::XORand source;

void sunflowerRecursive(int currentDepth, int depth, int breath, float lambda, float gamma, float distance, glm::vec3 position, std::vector<glm::vec3>& positions, int N) {
  if (positions.size() > N) return;
  if (currentDepth > depth) return;

  glm::vec3 v(
    source.randf() - 0.5f,
    source.randf() - 0.5f,
    source.randf() - 0.5f);

  glm::vec3 childPosition = v * lambda * distance + position;


  positions.push_back(childPosition);

  for (int i = 0; i < breath; i++) {
    sunflowerRecursive(currentDepth + 1, depth, breath, lambda / gamma, gamma, distance, childPosition, positions, N);
  }
}

std::vector<glm::vec3> sunflower(int depth, int breath, int N, float distance, float gamma) {
  std::vector<glm::vec3>   positions;
  random_sources::Standard source;
  sunflowerRecursive(0, depth, breath, 1.0f, gamma, distance, glm::vec3(0.0f), positions, N);
  return positions;
}
