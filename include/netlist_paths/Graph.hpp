#ifndef NETLIST_PATHS_GRAPH_HPP
#define NETLIST_PATHS_GRAPH_HPP

#include <algorithm>
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include "netlist_paths/DTypes.hpp"
#include "netlist_paths/Edge.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/InternalGraph.hpp"
#include "netlist_paths/Path.hpp"
#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

/// A class representing a netlist graph.
class Graph {
private:
  InternalGraph graph;
  std::map<std::string, VertexID> aliasMap;

  bool vertexTypeMatch(VertexID vertex, VertexNetlistType graphType) const;

  bool isAliasPath(const VertexIDVec &waypointIDs) const;

  VertexIDVec getAdjacentVerticesOutEdges(VertexID vertex) const;

  VertexIDVec getAdjacentVerticesInEdges(VertexID vertex) const;

  Path determinePath(ParentMap &parentMap,
                     Path path,
                     VertexID startVertexId,
                     VertexID endVertexId) const;

  void determineAllPaths(ParentMap &parentMap,
                         std::vector<Path> &result,
                         Path path,
                         VertexID startVertex,
                         VertexID endVertex) const;

public:
  Graph() {}

  //===--------------------------------------------------------------------===//
  // Graph construction methods.
  //===--------------------------------------------------------------------===//

  /// Add a Vertex object to the graph.
  VertexID addVertex(Vertex &vertex) {
    auto vertexID = boost::add_vertex(vertex, graph);
    graph[vertexID].setID(vertexID);
    return vertexID;
  }

  /// Add a logic vertex to the graph.
  VertexID addLogicVertex(VertexAstType type, Location location) {
    auto vertex = Vertex(type, location);
    auto vertexID = boost::add_vertex(vertex, graph);
    graph[vertexID].setID(vertexID);
    return vertexID;
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
    auto vertexID = boost::add_vertex(vertex, graph);
    graph[vertexID].setID(vertexID);
    return vertexID;
  }

  /// Add an edge to the graph. Don't allow multi-edges.
  void addEdge(VertexID src, VertexID dst) {
    if (!boost::edge(src, dst, graph).second) {
      boost::add_edge(src, dst, graph);
    }
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
  std::vector<Path> getAllFanOut(VertexID startVertex) const;

  /// Return a list of paths to an end vertex.
  std::vector<Path> getAllFanIn(VertexID endVertex) const;

  /// Count the fanout from a start vertex.
  size_t getfanOutDegree(VertexID startVertex);

  /// Count the fanin to an end vertex.
  size_t getFanInDegree(VertexID endVertex);

  /// Return any path between the specified waypoints, avoiding the specified
  /// mid points.
  Path getAnyPointToPoint(const VertexIDVec &waypointIDs,
                          const VertexIDVec &avoidPointIDs) const;

  /// Return all paths between the specified waypoints, avoiding the specified
  /// mid points.
  std::vector<Path> getAllPointToPoint(const VertexIDVec &waypoints,
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
