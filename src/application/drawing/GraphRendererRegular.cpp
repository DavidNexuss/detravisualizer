#include "GraphRenderer.hpp"
#include <mini/graphics.hpp>
#include <mini/io.hpp>

static const char* nodeVS = "assets/shaders/graph/regular/nodes.vert";
static const char* nodeFS = "assets/shaders/graph/regular/nodes.frag";
static const char* edgeVS = "assets/shaders/graph/regular/edges.vert";
static const char* edgeFS = "assets/shaders/graph/regular/edges.frag";

void pushLine(std::vector<glm::vec3>& mesh, glm::vec3 start, glm::vec3 end, float thickness) {
  glm::vec3 up = glm::cross(glm::normalize(end - start), glm::vec3(0, 0, 1)) * thickness * 0.5f;

  uint64_t top = mesh.size();

  mesh.resize(mesh.size() + 6);

  glm::vec3* buff = &mesh[top];

  buff[0] = start - up;
  buff[1] = end + up;
  buff[2] = start + up;

  buff[3] = end + up;
  buff[4] = end - up;
  buff[5] = start - up;
}

struct GraphRendererRegular : public GraphRenderer {
  GraphRendererConfiguration configuration;

  GraphRendererRegular() {
    initialized = false;
  }

  GraphRendererEntity entity;
  bool                initialized;
  GLuint              nodeVAO;
  GLuint              nodeVBO;
  GLuint              edgeVAO;
  GLuint              edgeVBO;
  GLuint              nodeProgram;
  GLuint              edgeProgram;

  GLuint uNodeViewProj;
  GLuint uNodeCenter;
  GLuint uNodeRadius;
  GLuint uNodeColor;
  GLuint uCameraPos;
  GLuint uEdgeViewProj;
  GLuint uEdgeColor;

  uint64_t vertexCountEdge;
  uint64_t vertexCountNode;

  bool confRenderEdges;
  bool confRenderNodes;

  void initNodeVao() {
    glGenVertexArrays(1, &nodeVAO);
    glGenBuffers(1, &nodeVBO);

    glBindVertexArray(nodeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, nodeVBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
  }

  void initEdgeVao() {
    glGenVertexArrays(1, &edgeVAO);
    glGenBuffers(1, &edgeVBO);

    glBindVertexArray(edgeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
  }

  void initPrograms() {
    nodeProgram = graphics::loadProgram(nodeFS, nodeVS);
    edgeProgram = graphics::loadProgram(edgeFS, edgeVS);

    // Node program uniforms
    uNodeViewProj = glGetUniformLocation(nodeProgram, "uViewProj");
    uNodeCenter   = glGetUniformLocation(nodeProgram, "uCenter");
    uNodeRadius   = glGetUniformLocation(nodeProgram, "uRadius");
    uNodeColor    = glGetUniformLocation(nodeProgram, "uColor");
    uCameraPos    = glGetUniformLocation(nodeProgram, "uCameraPos");

    // Edge program uniforms
    uEdgeViewProj = glGetUniformLocation(edgeProgram, "uViewProj");
    uEdgeColor    = glGetUniformLocation(edgeProgram, "uColor");
  }

  virtual void init() override {

    initialized = true;
    initPrograms();
    initNodeVao();
    initEdgeVao();
  }

  void buildNodes() {
    glBindBuffer(GL_ARRAY_BUFFER, nodeVBO);
    glBufferData(GL_ARRAY_BUFFER, entity.layout->positions.size() * sizeof(glm::vec3), entity.layout->positions.data(), GL_DYNAMIC_DRAW);

    vertexCountNode = entity.layout->positions.size();
  }

  void buildEdges() {
    std::vector<bool>      processed(entity.graph->getVertexCount());
    std::vector<glm::vec3> mesh;

    for (int u = 0; u < entity.graph->getVertexCount(); u++) {
      processed[u]  = true;
      auto edgelist = entity.graph->getEdges(u);

      for (int v : edgelist) {
        if (processed[v]) continue;
        pushLine(mesh, entity.layout->positions[u], entity.layout->positions[v], configuration.lineThickness);
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(glm::vec3), mesh.data(), GL_DYNAMIC_DRAW);

    vertexCountEdge = mesh.size();
  }

  void remesh() override {
    buildNodes();
    buildEdges();
  }

  void configure(GraphRendererConfiguration configuration) override {
    this->configuration = configuration;
  }

  virtual void render(GraphRendererEntity entity, Camera& camera, const glm::mat4& transform) override {
    if (this->entity != entity) {
      this->entity = entity;
      remesh();
    }

    if (io::fileHasChanged(nodeFS) || io::fileHasChanged(nodeVS) ||
        io::fileHasChanged(edgeFS) || io::fileHasChanged(edgeVS)) {
      try {
        initPrograms();
      } catch (...) {}
    }


    glm::mat4 mvp;

    mvp = camera.combined * transform;

    if (confRenderEdges && vertexCountEdge > 0) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glUseProgram(edgeProgram);
      glBindVertexArray(edgeVAO);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glUniformMatrix4fv(uEdgeViewProj, 1, GL_FALSE, &mvp[0][0]);
      glUniform3f(uEdgeColor, 0.7f, 0.7f, 0.7f);

      glDrawArrays(GL_TRIANGLES, 0, vertexCountEdge);
    }

    if (confRenderNodes && vertexCountNode > 0) {

      glUseProgram(nodeProgram);
      glBindVertexArray(nodeVAO);

      glEnable(GL_PROGRAM_POINT_SIZE);
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

      glUniformMatrix4fv(uNodeViewProj, 1, GL_FALSE, &mvp[0][0]);

      glm::vec3 camPos = camera.getCameraPosition();

      glUniform3fv(uCameraPos, 1, &camPos[0]);

      glDrawArrays(GL_POINTS, 0, vertexCountNode);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDisable(GL_BLEND);
    }
  }

  void destroy() {
    if (!initialized) return;
    glDeleteBuffers(1, &nodeVBO);
    glDeleteVertexArrays(1, &nodeVAO);
    glDeleteBuffers(1, &edgeVBO);
    glDeleteVertexArrays(1, &edgeVAO);

    glDeleteProgram(nodeProgram);
    glDeleteProgram(edgeProgram);
  }

  ~GraphRendererRegular() {
    destroy();
  }


  // This was intended as a programtic way to compute the edge overdraw of the current display so measurement could be objective instead of subjective

  /*
  struct EdgeOverdrawStats {
    uint64_t totalOverdraw = 0;
    uint32_t maxOverdraw   = 0;
  };

  EdgeOverdrawStats computeEdgeOverdraw(
    GLuint           edgeProgram,
    GLuint           edgeVAO,
    uint32_t         vertexCount,
    const glm::mat4& viewProj,
    int              framebufferWidth,
    int              framebufferHeight) {
    EdgeOverdrawStats stats;

    GLboolean colorMask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);

    GLboolean depthTest   = glIsEnabled(GL_DEPTH_TEST);
    GLboolean stencilTest = glIsEnabled(GL_STENCIL_TEST);

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glStencilMask(0xFF);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glUseProgram(edgeProgram);
    glBindVertexArray(edgeVAO);

    GLint uViewProj = glGetUniformLocation(edgeProgram, "uViewProj");
    if (uViewProj >= 0)
      glUniformMatrix4fv(uViewProj, 1, GL_FALSE, &viewProj[0][0]);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    std::vector<uint8_t> stencil(framebufferWidth * framebufferHeight);

    glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencil.data());

    for (uint8_t v : stencil) {
      stats.totalOverdraw += v;
      stats.maxOverdraw = std::max(stats.maxOverdraw, uint32_t(v));
    }

    glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);

    if (!depthTest) glDisable(GL_DEPTH_TEST);
    if (!stencilTest) glDisable(GL_STENCIL_TEST);

    return stats;
  }
  */
};



std::shared_ptr<GraphRenderer> createRegularRenderer() {
  return std::shared_ptr<GraphRenderer>(new GraphRendererRegular);
}
