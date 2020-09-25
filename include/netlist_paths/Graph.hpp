#ifndef NETLIST_PATHS_GRAPH_HPP
#define NETLIST_PATHS_GRAPH_HPP

#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/tokenizer.hpp>
#include "netlist_paths/DTypes.hpp"
#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

using InternalGraph = boost::adjacency_list<boost::vecS,
                                            boost::vecS,
                                            boost::bidirectionalS,
                                            Vertex>;
using VertexID = boost::graph_traits<InternalGraph>::vertex_descriptor;
using ParentMap = std::map<VertexID, std::vector<VertexID>>;
using VertexIDVec = std::vector<VertexID>;

class Graph {
private:
  InternalGraph graph;

  void dumpPath(const VertexIDVec &path) const;
  VertexIDVec determinePath(ParentMap &parentMap,
                            VertexIDVec path,
                            VertexID startVertexId,
                            VertexID endVertexId) const;
  void determineAllPaths(ParentMap &parentMap,
                         std::vector<VertexIDVec> &result,
                         VertexIDVec path,
                         VertexID startVertex,
                         VertexID endVertex) const;

public:
  Graph() {}
  VertexID addLogicVertex(VertexAstType type, Location location) {
    auto vertex = Vertex(type, location);
    return boost::add_vertex(vertex, graph);
  }
  VertexID addVarVertex(VertexAstType type,
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
  void addEdge(VertexID src, VertexID dst) {
    boost::add_edge(src, dst, graph);
  }
  void setVertexDstReg(VertexID vertex) {
    graph[vertex].setDstReg();
  }
  void setVertexDirection(VertexID vertex, VertexDirection direction) {
    graph[vertex].setDirection(direction);
  }
  VertexID nullVertex() const { return boost::graph_traits<InternalGraph>::null_vertex(); }
  std::size_t numVertices() { return boost::num_vertices(graph); }
  std::size_t numEdges() { return boost::num_edges(graph); }
  void splitRegVertices();
  void checkGraph() const;
  void dumpDotFile(const std::string &outputFilename) const;
  std::vector<VertexID> getAllVertices() const;
  VertexID getVertexDesc(const std::string &name) const;
  VertexID getVertexDescRegex(const std::string &name,
                                VertexGraphType graphType) const;
  VertexID getStartVertex(const std::string &name) const {
    return getVertexDescRegex(name, VertexGraphType::START_POINT);
  }
  VertexID getEndVertex(const std::string &name) const {
    return getVertexDescRegex(name, VertexGraphType::END_POINT);
  }
  VertexID getMidVertex(const std::string &name) const {
    return getVertexDescRegex(name, VertexGraphType::MID_POINT);
  }
  VertexID getRegVertex(const std::string &name) const {
    return getVertexDescRegex(name, VertexGraphType::REG);
  }
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
  VertexIDVec getAnyPointToPoint(const std::vector<VertexID> &waypoints) const;
  //std::vector<Path> getAllPointToPoint() const;
  const Vertex &getVertex(VertexID vertexId) const { return graph[vertexId]; }
  Vertex* getVertexPtr(VertexID vertexId) const {
    // Remove the const cast to make it compatible with the boost::python wrappers.
    return const_cast<Vertex*>(&(graph[vertexId]));
  }
};

} // End namespace.

#endif // NETLIST_PATHS_GRAPH_HPP
