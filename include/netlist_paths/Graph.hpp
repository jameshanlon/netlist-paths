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
#include "netlist_paths/Edge.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

using InternalGraph = boost::adjacency_list<boost::vecS,
                                            boost::vecS,
                                            boost::bidirectionalS,
                                            Vertex,
                                            Edge>;
using VertexID = boost::graph_traits<InternalGraph>::vertex_descriptor;
using EdgeID = boost::graph_traits<InternalGraph>::edge_descriptor;
using ParentMap = std::map<VertexID, std::vector<VertexID>>;
using VertexIDVec = std::vector<VertexID>;

/// An edge predicate for the filtered graph.
struct EdgePredicate {

  const InternalGraph *graph;

  EdgePredicate() {}

  EdgePredicate(const InternalGraph *graph) : graph(graph) {}

  bool operator()(EdgeID edgeID) const {
    // Include all edges if traversal of registers is enabled, otherwise only
    // include edges that do not traverse a register.
    if (Options::getInstance().shouldTraverseRegisters()) {
      return true;
    } else {
      return !(*graph)[edgeID].isThroughRegister();
    }
  }
};

/// A vertex predicate for the filtered graph.
struct VertexPredicate {
  const VertexIDVec *avoidPointIDs;

  VertexPredicate() : avoidPointIDs(nullptr) {}

  VertexPredicate(const VertexIDVec *avoidPointIDs) :
      avoidPointIDs(avoidPointIDs) {}

  /// Return true if the vertex should be excluded from the search.
  bool operator()(VertexID vertexID) const {
    if (!avoidPointIDs) {
      return true;
    } else {
      return !std::binary_search(avoidPointIDs->begin(), avoidPointIDs->end(),
                                 vertexID);
    }
  }
};

using FilteredInternalGraph = boost::filtered_graph<InternalGraph,
                                                    EdgePredicate,
                                                    VertexPredicate>;

/// A class representing a netlist graph.
class Graph {
private:
  InternalGraph graph;
  std::map<std::string, VertexID> aliasMap;

  bool vertexTypeMatch(VertexID vertex, VertexNetlistType graphType) const;

  bool isAliasPath(const VertexIDVec &waypointIDs) const;

  VertexIDVec getAdjacentVerticesOutEdges(VertexID vertex) const;

  VertexIDVec getAdjacentVerticesInEdges(VertexID vertex) const;

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

  //===--------------------------------------------------------------------===//
  // Graph construction methods.
  //===--------------------------------------------------------------------===//

  /// Add a logic vertex to the graph.
  VertexID addLogicVertex(VertexAstType type, Location location) {
    auto vertex = Vertex(type, location);
    return boost::add_vertex(vertex, graph);
  }

  /// Add a variable vertex to the graph.
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

  /// Add an edge to the graph.
  void addEdge(VertexID src, VertexID dst) {
    boost::add_edge(src, dst, graph);
  }

  /// Set the specified vertex to be a destination register.
  void setVertexDstReg(VertexID vertex) {
    graph[vertex].setDstReg();
  }

  /// Set the direction of the specified vertex.
  void setVertexDirection(VertexID vertex, VertexDirection direction) {
    graph[vertex].setDirection(direction);
  }

  /// Mark all variables that are aliases of registers.
  void markAliasRegisters();

  /// Split register vertices into source and destination parts.
  void splitRegVertices();

  /// Add additional edges to variable aliases.
  void updateVarAliases();

  /// Perform some checks on the final graph.
  void checkGraph() const;

  /// Write a dot file of the graph to file.
  void dumpDotFile(const std::string &outputFilename) const;

  //===--------------------------------------------------------------------===//
  // Vertex access.
  //===--------------------------------------------------------------------===//

  /// Return a list of vertices matching a VertexGraphType.
  VertexIDVec getVerticesByType(VertexNetlistType graphType) const;

  /// Lookup a vertex by matching its name exactly.
  VertexID getVertexExact(const std::string &name,
                          VertexNetlistType graphType=VertexNetlistType::ANY) const;

  /// Return a list of vertices matching a regex pattern.
  VertexIDVec getVerticesRegex(const std::string &pattern,
                               VertexNetlistType graphType=VertexNetlistType::ANY) const;

  /// Return a list of vertices matching a wildcard pattern.
  VertexIDVec getVerticesWildcard(const std::string &pattern,
                                  VertexNetlistType graphType=VertexNetlistType::ANY) const;

  /// Return a list of vertices that match the pattern.
  VertexIDVec getVertices(const std::string &pattern,
                          VertexNetlistType graphType=VertexNetlistType::ANY) const;

  /// Specialisation of getVertices for startpoints.
  VertexIDVec getStartVertices(const std::string &name) const {
    return getVertices(name, VertexNetlistType::START_POINT);
  }

  /// Specialisation of getVertices for endpoints.
  VertexIDVec getEndVertices(const std::string &name) const {
    return getVertices(name, VertexNetlistType::END_POINT);
  }

  /// Specialisation of getVertices for midpoints.
  VertexIDVec getMidVertices(const std::string &name) const {
    return getVertices(name, VertexNetlistType::MID_POINT);
  }

  /// Specialisation of getVertices for registers.
  VertexIDVec getRegVertices(const std::string &name) const {
    return getVertices(name, VertexNetlistType::REG);
  }

  /// Specialisation of getVertices for register aliases.
  VertexIDVec getRegAliasVertices(const std::string &name) const {
    return getVertices(name, VertexNetlistType::DST_REG_ALIAS);
  }

  //===--------------------------------------------------------------------===//
  // Path access.
  //===--------------------------------------------------------------------===//

  /// Return a list of paths from a start vertex.
  std::vector<VertexIDVec> getAllFanOut(VertexID startVertex) const;

  /// Return a list of paths to an end vertex.
  std::vector<VertexIDVec> getAllFanIn(VertexID endVertex) const;

  /// Count the fanout from a start vertex.
  size_t getfanOutDegree(VertexID startVertex);

  /// Count the fanin to an end vertex.
  size_t getFanInDegree(VertexID endVertex);

  /// Return any path between the specified waypoints, avoiding the specified
  /// mid points.
  VertexIDVec getAnyPointToPoint(const VertexIDVec &waypointIDs,
                                 const VertexIDVec &avoidPointIDs) const;

  /// Return all paths between the specified waypoints, avoiding the specified
  /// mid points.
  std::vector<VertexIDVec> getAllPointToPoint(const VertexIDVec &waypoints,
                                              const VertexIDVec &avoidPointIDs) const;

  //===--------------------------------------------------------------------===//
  // Miscellaneous getters and setters.
  //===--------------------------------------------------------------------===//

  const Vertex &getVertex(VertexID vertexId) const { return graph[vertexId]; }

  Vertex* getVertexPtr(VertexID vertexId) const {
    // Remove the const cast to make it compatible with the boost::python wrappers.
    return const_cast<Vertex*>(&(graph[vertexId]));
  }

  VertexID nullVertex() const { return boost::graph_traits<InternalGraph>::null_vertex(); }
  std::size_t numVertices() const { return boost::num_vertices(graph); }
  std::size_t numEdges() const { return boost::num_edges(graph); }
};

} // End namespace.

#endif // NETLIST_PATHS_GRAPH_HPP
