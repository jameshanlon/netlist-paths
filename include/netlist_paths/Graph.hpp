#ifndef NETLIST_PATHS_GRAPH_HPP
#define NETLIST_PATHS_GRAPH_HPP

#include <algorithm>
#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/tokenizer.hpp>
#include "netlist_paths/DTypes.hpp"
#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

using InternalGraph = boost::adjacency_list<boost::vecS,
                                            boost::vecS,
                                            boost::bidirectionalS,
                                            Vertex>;
using VertexID = boost::graph_traits<InternalGraph>::vertex_descriptor;
using EdgeID = boost::graph_traits<InternalGraph>::edge_descriptor;
using ParentMap = std::map<VertexID, std::vector<VertexID>>;
using VertexIDVec = std::vector<VertexID>;

/// A vertex predicate for the filtered graph.
struct VertexPredicate {
  const VertexIDVec *avoidPointIDs;
  VertexPredicate() {}
  VertexPredicate(const VertexIDVec *avoidPointIDs) :
      avoidPointIDs(avoidPointIDs) {}
  /// Return true if the vertex should be excluded from the search.
  bool operator()(VertexID vertexID) const {
    return !std::binary_search(avoidPointIDs->begin(), avoidPointIDs->end(),
                               vertexID);
  }
};

using FilteredInternalGraph = boost::filtered_graph<InternalGraph,
                                                    boost::keep_all,
                                                    VertexPredicate>;

/// A class representing a netlist graph.
class Graph {
private:
  InternalGraph graph;

  bool vertexTypeMatch(VertexID vertex, VertexGraphType graphType) const;

  VertexIDVec getTargetNodes(VertexID vertex) const;

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
  std::size_t numVertices() const { return boost::num_vertices(graph); }
  std::size_t numEdges() const { return boost::num_edges(graph); }
  void markAliasRegisters();
  void splitRegVertices();
  void checkGraph() const;
  void dumpDotFile(const std::string &outputFilename) const;

  /// Return a list of vertices matching a VertexGraphType.
  VertexIDVec getVerticesByType(VertexGraphType graphType) const;

  /// Lookup a vertex by matching its name exactly.
  VertexID getVertexExact(const std::string &name,
                          VertexGraphType graphType=VertexGraphType::ANY) const;

  /// Return a list of vertices matching a regex pattern.
  VertexIDVec getVerticesRegex(const std::string &pattern,
                               VertexGraphType graphType=VertexGraphType::ANY) const;

  /// Return a list of vertices matching a wildcard pattern.
  VertexIDVec getVerticesWildcard(const std::string &pattern,
                                  VertexGraphType graphType=VertexGraphType::ANY) const;

  /// Return a list of vertices that match the pattern.
  VertexIDVec getVertices(const std::string &pattern,
                          VertexGraphType graphType=VertexGraphType::ANY) const;

  VertexIDVec getStartVertices(const std::string &name) const {
    return getVertices(name, VertexGraphType::START_POINT);
  }
  VertexIDVec getEndVertices(const std::string &name) const {
    return getVertices(name, VertexGraphType::END_POINT);
  }
  VertexIDVec getMidVertices(const std::string &name) const {
    return getVertices(name, VertexGraphType::MID_POINT);
  }
  VertexIDVec getRegVertices(const std::string &name) const {
    return getVertices(name, VertexGraphType::REG);
  }
  VertexIDVec getRegAliasVertices(const std::string &name) const {
    return getVertices(name, VertexGraphType::REG_ALIAS);
  }
  std::vector<VertexIDVec> getAllFanOut(VertexID startVertex) const;
  std::vector<VertexIDVec> getAllFanIn(VertexID endVertex) const;
  size_t getfanOutDegree(VertexID startVertex);
  size_t getFanInDegree(VertexID endVertex);
  VertexIDVec getAnyPointToPoint(const VertexIDVec &waypointIDs,
                                 const VertexIDVec &avoidPointIDs) const;
  std::vector<VertexIDVec> getAllPointToPoint(const VertexIDVec &waypoints,
                                              const VertexIDVec &avoidPointIDs) const;
  const Vertex &getVertex(VertexID vertexId) const { return graph[vertexId]; }
  Vertex* getVertexPtr(VertexID vertexId) const {
    // Remove the const cast to make it compatible with the boost::python wrappers.
    return const_cast<Vertex*>(&(graph[vertexId]));
  }
};

} // End namespace.

#endif // NETLIST_PATHS_GRAPH_HPP
