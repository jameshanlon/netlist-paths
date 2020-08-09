#ifndef NETLIST_PATHS_NETLIST_HPP
#define NETLIST_PATHS_NETLIST_HPP

#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/tokenizer.hpp>
#include "netlist_paths/DTypes.hpp"
#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

using Graph = boost::adjacency_list<boost::vecS,
                                    boost::vecS,
                                    boost::bidirectionalS,
                                    Vertex>;
using VertexDesc = boost::graph_traits<Graph>::vertex_descriptor;
using ParentMap = std::map<VertexDesc, std::vector<VertexDesc>>;
using Path = std::vector<VertexDesc>;

class Netlist {
private:
  Graph graph;

  void dumpPath(const Path &path) const;
  Path determinePath(ParentMap &parentMap,
                     Path path,
                     VertexDesc startVertexId,
                     VertexDesc endVertexId) const;
  void determineAllPaths(ParentMap &parentMap,
                         std::vector<Path> &result,
                         Path path,
                         VertexDesc startVertex,
                         VertexDesc endVertex) const;

public:
  Netlist() {}
  VertexDesc addLogicVertex(VertexAstType type, Location location) {
    auto vertex = Vertex(type, location);
    return boost::add_vertex(vertex, graph);
  }
  VertexDesc addVarVertex(VertexAstType type,
                          VertexDirection direction,
                          Location location,
                          std::shared_ptr<DType> dtype,
                          const std::string &name,
                          bool isParam,
                          const std::string &paramValue,
                          bool isPublic) {
    auto vertex = Vertex(type, direction, location, dtype, name, isParam,
                         paramValue, isPublic);
    return boost::add_vertex(vertex, graph);
  }
  void addEdge(VertexDesc src, VertexDesc dst) {
    boost::add_edge(src, dst, graph);
  }
  void setVertexDstReg(VertexDesc vertex) {
    graph[vertex].astType = VertexAstType::DST_REG;
  }
  void setVertexDirection(VertexDesc vertex, VertexDirection direction) {
    graph[vertex].direction = direction;
  }
  /// Annotate source register vertices.
  //void annotateRegVertices();
  VertexDesc nullVertex() const { return boost::graph_traits<Graph>::null_vertex(); }
  std::size_t numVertices() { return boost::num_vertices(graph); }
  std::size_t numEdges() { return boost::num_edges(graph); }
  void splitRegVertices();
  void mergeDuplicateVertices();
  void checkGraph() const;
  void dumpDotFile(const std::string &outputFilename) const;
  std::vector<VertexDesc> getAllVertices() const;
  VertexDesc getVertexDesc(const std::string &name) const;
  VertexDesc getVertexDescRegex(const std::string &name,
                                VertexGraphType graphType) const;
  VertexDesc getStartVertex(const std::string &name) const {
    return getVertexDescRegex(name, VertexGraphType::START_POINT);
  }
  VertexDesc getEndVertex(const std::string &name) const {
    return getVertexDescRegex(name, VertexGraphType::END_POINT);
  }
  VertexDesc getMidVertex(const std::string &name) const {
    return getVertexDescRegex(name, VertexGraphType::MID_POINT);
  }
  VertexDesc getRegVertex(const std::string &name) const {
    return getVertexDescRegex(name, VertexGraphType::REG);
  }
  //VertexDesc getStartVertexExcept(const std::string &name) const;
  //VertexDesc getEndVertexExcept(const std::string &name) const;
  //VertexDesc getMidVertexExcept(const std::string &name) const;
  //std::vector<Path> getAllFanOut(VertexDesc startVertex) const;
  //std::vector<Path> getAllFanOut(const std::string &startName) const {
  //  auto startVertex = getStartVertexExcept(startName);
  //  return getAllFanOut(startVertex);
  //}
  //std::vector<Path> getAllFanIn(VertexDesc endVertex) const;
  //std::vector<Path> getAllFanIn(const std::string &endName) const {
  //  auto endVertex = getEndVertexExcept(endName);
  //  return getAllFanIn(endVertex);
  //}
  //unsigned getfanOutDegree(VertexDesc startVertex);
  //unsigned getfanOutDegree(const std::string &startName);
  //  auto startVertex = getStartVertexExcept(startName);
  //  return getfanOutDegree(startVertex);
  //}
  //unsigned getFanInDegree(VertexDesc endVertex);
  //unsigned getFanInDegree(const std::string &endName);
  //  auto endVertex = getEndVertexExcept(endName);
  //  return getFanInDegree(endVertex);
  //}
  Path getAnyPointToPoint(const std::vector<VertexDesc> &waypoints) const;
  //std::vector<Path> getAllPointToPoint() const;
  const Vertex &getVertex(VertexDesc vertexId) const { return graph[vertexId]; }
};

} // End namespace.

#endif // NETLIST_PATHS_NETLIST_HPP
