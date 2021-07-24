#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <regex>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/tokenizer.hpp>
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Graph.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/Utilities.hpp"

using namespace netlist_paths;

class DfsVisitor : public boost::default_dfs_visitor {
private:
  ParentMap &parentMap;
  bool allPaths;
public:
  DfsVisitor(ParentMap &parentMap, bool allPaths) :
      parentMap(parentMap), allPaths(allPaths) {}
  // Visit only the edges of the DFS graph.
  template<typename Edge, typename Graph>
  void tree_edge(Edge edge, const Graph &graph) const {
    if (!allPaths) {
      VertexID src, dst;
      src = boost::source(edge, graph);
      dst = boost::target(edge, graph);
      parentMap[dst].push_back(src);
    }
    return;
  }
  // Visit all edges of a vertex.
  template<typename Edge, typename Graph>
  void examine_edge(Edge edge, const Graph &graph) const {
    if (allPaths) {
      VertexID src, dst;
      src = boost::source(edge, graph);
      dst = boost::target(edge, graph);
      parentMap[dst].push_back(src);
    }
    return;
  }
};

/// Get all vertices connected by out edges from vertex.
VertexIDVec Graph::getAdjacentVerticesOutEdges(VertexID vertex) const {
  std::vector<VertexID> targets;
  BGL_FORALL_OUTEDGES(vertex, outEdge, graph, InternalGraph) {
    targets.push_back(boost::target(outEdge, graph));
  }
  return targets;
}

/// Get all vertices connected by in edges from vertex.
VertexIDVec Graph::getAdjacentVerticesInEdges(VertexID vertex) const {
  std::vector<VertexID> sources;
  BGL_FORALL_INEDGES(vertex, outEdge, graph, InternalGraph) {
    sources.push_back(boost::source(outEdge, graph));
  }
  return sources;
}

/// For any register vertex that is alias assigned to another variable, mark
/// that variable as being an alias of the register. This fixes issues with the
/// way that Verilator inlines modules, ensuring that the target variable of a
/// delayed assignment is always correctly marked as a register.
void Graph::markAliasRegisters() {
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (graph[v].isReg()) {
      for (auto target : getAdjacentVerticesOutEdges(v)) {
        if (graph[target].getAstType() == VertexAstType::ASSIGN_ALIAS) {
          VertexIDVec assignAliasTargets = getAdjacentVerticesOutEdges(target);
          assert(assignAliasTargets.size() == 1);
          if (assignAliasTargets.front() != v) {
            graph[assignAliasTargets.front()].setDstRegAlias();
            BOOST_LOG_TRIVIAL(debug) << boost::format("Marked %s as REG alias of %s")
                % graph[assignAliasTargets.front()].getName() % graph[v].getName();
            // Create a mapping of the alias name to the (destination) register vertex ID.
            aliasMap[graph[assignAliasTargets.front()].getName()] = v;
          }
        }
      }
    }
  }
}

/// Register vertices are split into 'destination' registers only with in edges
/// and 'source' registers only with out edges. This implies graph connectivity
/// follows combinatorial paths in the netlist and allows traversals of the
/// graph to trace combinatorial timing paths.
void Graph::splitRegVertices() {
  // Create a list of vertices to iterate over since this transform adds new
  // vertices and would invalidate a vertex iterator.
  VertexIDVec regVertices;
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (graph[v].isReg()) {
      regVertices.push_back(v);
    }
  }
  for (auto v : regVertices) {
    // Create a new 'source' reg vertex.
    Vertex srcReg(graph[v]);
    srcReg.setSrcReg();
    auto srcRegVertex = boost::add_vertex(srcReg, graph);
    // Collect all adjacent vertices (to which there are out edges).
    std::vector<VertexID> adjacentVertices = getAdjacentVerticesOutEdges(v);
    // Create or move edges to src register vertex.
    for (auto target : adjacentVertices) {

      // Handle ASSIGN_ALIAS nodes.
      if (graph[target].getAstType() == VertexAstType::ASSIGN_ALIAS) {
        VertexIDVec assignAliasTargets = getAdjacentVerticesOutEdges(target);
        assert(assignAliasTargets.size() == 1);
        if (assignAliasTargets.front() != v) {
          // If we have REG -> ASSIGN_ALIAS -> VAR, then duplicate
          // ASSIGN_ALIAS and VAR, so there are:
          //   SRC_REG <- ASSIGN_ALIAS <- VAR
          //   DST_REG -> ASSIGN_ALIAS -> VAR
          Vertex assignAlias(graph[target]);
          Vertex aliasVar(graph[assignAliasTargets.front()]);
          aliasVar.setSrcRegAlias();
          auto assignAliasVertex = boost::add_vertex(assignAlias, graph);
          auto aliasVarVertex = boost::add_vertex(aliasVar, graph);
          boost::add_edge(aliasVarVertex, assignAliasVertex, graph);
          boost::add_edge(assignAliasVertex, srcRegVertex, graph);
          continue;
        }
      }

      // Otherwise, mark edges from a DST_REG node to each node that is
      // connected by an out edge, allowing paths through registered to be
      // traversed.
      graph[boost::edge(v, target, graph).first].setThroughRegister();

      // And copy the same out edge to the new srcReg.
      boost::add_edge(srcRegVertex, target, graph);
    }
  }
}

/// Add additional edges from variable aliases, through the ASSIGN_ALIAS node,
/// to allow the alias variable to act as a start point.
void Graph::updateVarAliases() {
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (graph[v].getAstType() == VertexAstType::ASSIGN_ALIAS) {
      VertexIDVec sourceVertices = getAdjacentVerticesInEdges(v);
      VertexIDVec targetVertices = getAdjacentVerticesOutEdges(v);
      assert(targetVertices.size() == 1);
      VertexID assignAlias = v;
      VertexID aliasVar = targetVertices.front();
      for (auto sourceVertex : sourceVertices) {
        VertexID sourceVar = sourceVertex;
        if (!graph[sourceVar].isReg()) {
          // We have VAR -> ASSIGN_ALIAS -> VAR (alias)
          // Add back edges: VAR (alias) -> ASSIGN_ALIAS
          //                 ASSIGN_ALIAS -> VAR
          if (!boost::edge(aliasVar, assignAlias, graph).second) {
            boost::add_edge(aliasVar, assignAlias, graph);
          }
          if (!boost::edge(assignAlias, sourceVar, graph).second) {
            boost::add_edge(assignAlias, sourceVar, graph);
          }
        }
      }
    }
  }
}

/// Perform some checks on the netlist and emit warnings if necessary.
void Graph::checkGraph() const {
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    // Check there are no Vlvbound nodes.
    if (graph[v].getName().find("__Vlvbound") != std::string::npos) {
      BOOST_LOG_TRIVIAL(warning) << boost::format("%s vertex in netlist") % graph[v].toString();
    }
    // Source registers don't have in edges.
    if (graph[v].isSrcReg()) {
      if (boost::in_degree(v, graph) > 0) {
        BOOST_LOG_TRIVIAL(warning) << boost::format("source reg %s has in edges") % graph[v].toString();
      }
    }
    // Destination registers don't have out edges.
    if (graph[v].isDstReg()) {
      if (boost::out_degree(v, graph) > 0) {
        BOOST_LOG_TRIVIAL(warning) << boost::format("destination reg has out edges") % graph[v].toString();
      }
    }
    // NOTE: vertices may be incorrectly marked as reg if a field of a
    // structure has a delayed assignment to a field of it.
  }
}

/// Dump a Graphviz dotfile of the netlist graph for visualisation.
void Graph::dumpDotFile(const std::string &outputFilename) const {
  std::ofstream outputFile(outputFilename);
  if (!outputFile.is_open()) {
    throw Exception(std::string("unable to open ")+outputFilename);
  }
  FilteredInternalGraph filteredGraph(graph,
                                      EdgePredicate(&graph),
                                      VertexPredicate({}));
  // Loop over all vertices and print properties.
  outputFile << "digraph netlist {\n";
  BGL_FORALL_VERTICES(v, filteredGraph, FilteredInternalGraph) {
    outputFile << v << boost::format(" [label=\"%s %s\"]\n")
                    % graph[v].getName() % graph[v].getAstTypeStr();
  }
  // Loop over all edges.
  BGL_FORALL_EDGES(e, filteredGraph, FilteredInternalGraph) {
    outputFile << boost::format("%d -> %d;\n")
                    % boost::source(e, graph) % boost::target(e, graph);
  }
  outputFile << "}\n";
  outputFile.close();
  // Print command line to generate graph file.
  BOOST_LOG_TRIVIAL(info) << boost::format("dot -Tpdf %s -o graph.pdf") % outputFilename;
}

/// Match a VertexGraphType against a vertex.
bool Graph::vertexTypeMatch(VertexID vertex, VertexNetlistType graphType) const {
  if (graphType == VertexNetlistType::ANY) {
    return true;
  }
  if ((graphType == VertexNetlistType::REG ||
       graphType == VertexNetlistType::PORT ||
       graphType == VertexNetlistType::IS_NAMED) &&
      (graph[vertex].isSrcReg() ||
       graph[vertex].isSrcRegAlias())) {
    // Source registers and register aliases are duplicates of destination
    // registers and their aliases, so exclude them from queries that can
    // include registers.
    return false;
  }
  // Anything else is handled by isGraphType().
  return graph[vertex].isGraphType(graphType);
}

VertexIDVec Graph::getVerticesByType(VertexNetlistType graphType) const {
  VertexIDVec vertexIDs;
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (vertexTypeMatch(v, graphType)) {
      vertexIDs.push_back(v);
    }
  }
  return vertexIDs;
}

/// This implementation will allow the name to contain other regular expression
/// syntax, and should be improved to match the wildcards directly.
VertexIDVec Graph::getVerticesWildcard(const std::string &name,
                                       VertexNetlistType graphType) const {
  auto nameStr(name);
  if (Options::getInstance().shouldIgnoreHierarchyMarkers()) {
    // Ignore '/', '.' and '_' characters.
    std::replace(nameStr.begin(), nameStr.end(), '/', '?');
    std::replace(nameStr.begin(), nameStr.end(), '.', '?');
    std::replace(nameStr.begin(), nameStr.end(), '_', '?');
  }
  VertexIDVec vertexIDs;
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (vertexTypeMatch(v, graphType) &&
        wildcardMatch(graph[v].getName(), nameStr)) {
      vertexIDs.push_back(v);
    }
  }
  return vertexIDs;
}

VertexIDVec Graph::getVerticesRegex(const std::string &name,
                                    VertexNetlistType graphType) const {
  auto nameStr(name);
  if (Options::getInstance().shouldIgnoreHierarchyMarkers()) {
    // Ignore '/' or '_' ('.' already matches any character).
    std::replace(nameStr.begin(), nameStr.end(), '/', '.');
    std::replace(nameStr.begin(), nameStr.end(), '_', '.');
  }
  // Catch any errors in the regex string.
  std::regex nameRegex;
  try {
    nameRegex.assign(nameStr);
  } catch(std::regex_error const &e) {
    throw Exception(std::string("malformed regular expression: ")+e.what());
  }
  // Search the vertices.
  VertexIDVec vertexIDs;
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (vertexTypeMatch(v, graphType) &&
        std::regex_search(graph[v].getName(), nameRegex)) {
      vertexIDs.push_back(v);
    }
  }
  return vertexIDs;
}

VertexID Graph::getVertexExact(const std::string &name,
                               VertexNetlistType graphType) const {
  if (Options::getInstance().shouldIgnoreHierarchyMarkers()) {
    // If hierarchy markers should be ignored, then use regex matching, but
    // constrain the supplied name to match exactly and expect only one matching
    // vertex.
    auto matchingVertices = getVerticesRegex(boost::str(boost::format("^%s$") % name),
                                             graphType);
    if (matchingVertices.size() > 1) {
      throw Exception(std::string("malformed exact pattern: ") + name);
    }
    if (matchingVertices.size() == 0) {
      return nullVertex();
    }
    return matchingVertices.front();
  }
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (vertexTypeMatch(v, graphType) &&
        graph[v].getName() == name) {
      return v;
    }
  }
  return nullVertex();
}

VertexIDVec Graph::getVertices(const std::string &pattern,
                               VertexNetlistType graphType) const {
  if (pattern.empty()) {
    return getVerticesByType(graphType);
  }
  if (Options::getInstance().isMatchExact()) {
    auto vertex = getVertexExact(pattern, graphType);
    return vertex != nullVertex() ? VertexIDVec{vertex} : VertexIDVec{};
  }
  if (Options::getInstance().isMatchRegex()) {
    return getVerticesRegex(pattern, graphType);
  }
  if (Options::getInstance().isMatchWildcard()) {
    return getVerticesWildcard(pattern, graphType);
  }
  return {};
}

/// Given the tree structure from a DFS, traverse the tree from leaf to root to
/// return a path.
VertexIDVec Graph::determinePath(ParentMap &parentMap,
                                 VertexIDVec path,
                                 VertexID startVertex,
                                 VertexID finishVertex) const {
  path.push_back(finishVertex);
  if (finishVertex == startVertex) {
    return path;
  }
  if (parentMap[finishVertex].empty()) {
    return VertexIDVec();
  }
  assert(parentMap[finishVertex].size() == 1);
  auto nextVertex = parentMap[finishVertex].front();
  assert(std::find(std::begin(path),
                   std::end(path),
                   nextVertex) == std::end(path));
  return determinePath(parentMap, path, startVertex, nextVertex);
}

/// Determine all paths between a start and an end point.
/// This performs a DFS starting at the end point. It is not feasible for large
/// graphs since the number of simple paths grows exponentially.
void Graph::determineAllPaths(ParentMap &parentMap,
                              std::vector<VertexIDVec> &result,
                              VertexIDVec path,
                              VertexID startVertex,
                              VertexID finishVertex) const {
  path.push_back(finishVertex);
  if (finishVertex == startVertex) {
    BOOST_LOG_TRIVIAL(debug) << "Found path";
    result.push_back(path);
    return;
  }
  BOOST_LOG_TRIVIAL(debug) << boost::format("length %d vertex %s")
                                % path.size() % graph[finishVertex].toString();
  // Dump path.
  if (Options::getInstance().isDebugMode()) {
    for (auto v : path) {
      if (!graph[v].isLogic()) {
        BOOST_LOG_TRIVIAL(debug) << " " << graph[v].getName();
      }
    }
  }
  BOOST_LOG_TRIVIAL(debug) << (parentMap[finishVertex].empty() ? "DEAD END" : "");
  for (auto vertex : parentMap[finishVertex]) {
    if (std::find(std::begin(path), std::end(path), vertex) == std::end(path)) {
      determineAllPaths(parentMap, result, path, startVertex, vertex);
    } else {
      BOOST_LOG_TRIVIAL(debug) << "Cycle detected";
    }
  }
}

/// Report all paths fanning out from a net/register/port.
std::vector<VertexIDVec>
Graph::getAllFanOut(VertexID startVertex) const {
  FilteredInternalGraph filteredGraph(graph,
                                      EdgePredicate(&graph),
                                      VertexPredicate());
  BOOST_LOG_TRIVIAL(debug) << "Performing DFS from " << graph[startVertex].getName();
  ParentMap parentMap;
  boost::depth_first_search(filteredGraph,
      boost::visitor(DfsVisitor(parentMap, false))
        .root_vertex(startVertex));
  // Check for a path between startPoint and each register.
  std::vector<VertexIDVec> paths;
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (graph[v].isEndPoint()) {
      auto path = determinePath(parentMap,
                                VertexIDVec(),
                                startVertex,
                                static_cast<VertexID>(v));
      if (!path.empty()) {
        std::reverse(std::begin(path), std::end(path));
        paths.push_back(path);
      }
    }
  }
  return paths;
}

/// Report all paths fanning into a net/register/port.
std::vector<VertexIDVec>
Graph::getAllFanIn(VertexID finishVertex) const {
  FilteredInternalGraph filteredGraph(graph,
                                      EdgePredicate(&graph),
                                      VertexPredicate());
  auto reverseGraph = boost::make_reverse_graph(filteredGraph);
  BOOST_LOG_TRIVIAL(debug) << "Performing DFS in reverse graph from " << graph[finishVertex].getName();
  ParentMap parentMap;
  boost::depth_first_search(reverseGraph,
      boost::visitor(DfsVisitor(parentMap, false))
        .root_vertex(finishVertex));
  // Check for a path between endPoint and each register.
  std::vector<VertexIDVec> paths;
  BGL_FORALL_VERTICES(v, graph, InternalGraph) {
    if (graph[v].isStartPoint()) {
      auto path = determinePath(parentMap,
                                VertexIDVec(),
                                finishVertex,
                                static_cast<VertexID>(v));
      if (!path.empty()) {
        paths.push_back(path);
      }
    }
  }
  return paths;
}

/// Given a vector of vectors of paths (the set of all paths between each
/// through point), return a vector of paths that is the cartesian product of
/// the paths in each stage. Based on code in:
///   https://stackoverflow.com/questions/5279051/how-can-i-create-cartesian-product-of-vector-of-vectors
std::vector<VertexIDVec>
pathProduct(const std::vector<std::vector<VertexIDVec>>& intPaths) {
  std::vector<VertexIDVec> resultPaths = {{}};
  for (const auto& stagePaths : intPaths) {
    std::vector<VertexIDVec> newPaths;
    // Multiply each of the existing (sub) paths, with the next sub paths.
    // Ie for each path create a new path with the next sub path appended.
    for (const auto& resultPath : resultPaths) {
      for (const auto &subPath : stagePaths) {
        VertexIDVec path(subPath);
        std::reverse(path.begin(), path.end());
        // Append the new sub path to the existing 'resultPath'.
        newPaths.push_back(resultPath);
        newPaths.back().insert(newPaths.back().end(), path.begin(), path.end()-1);
      }
    }
    resultPaths = std::move(newPaths);
  }
  return resultPaths;
}

/// Return true if exactly two waypoints correspond to aliases of the same variable.
bool Graph::isAliasPath(const VertexIDVec &waypointIDs) const {
  if (waypointIDs.size() == 2 &&
      aliasMap.count(graph[waypointIDs[0]].getName()) &&
      aliasMap.count(graph[waypointIDs[1]].getName())) {
    return aliasMap.at(graph[waypointIDs[0]].getName()) ==
               aliasMap.at(graph[waypointIDs[1]].getName());
  }
  return false;
}

/// Report all paths between start and finish points.
/// Though points currently unsupported.
std::vector<VertexIDVec>
Graph::getAllPointToPoint(const VertexIDVec &waypointIDs,
                          const VertexIDVec &avoidPointIDs) const {
  // Special case for paths between aliases of the same variable.
  if (isAliasPath(waypointIDs)) {
    BOOST_LOG_TRIVIAL(debug) << boost::format("%s is alias of %s")
                                  % graph[waypointIDs[0]].getName()
                                  % graph[waypointIDs[1]].getName();
    return {{waypointIDs[0], waypointIDs[1]}};
  }
  FilteredInternalGraph filteredGraph(graph,
                                      EdgePredicate(&graph),
                                      VertexPredicate(&avoidPointIDs));
  std::vector<std::vector<VertexIDVec> > intPaths;
  // Elaborate all paths between each adjacent waypoint.
  for (std::size_t i = 0; i < waypointIDs.size()-1; ++i) {
    auto beginVertex = waypointIDs[i];
    auto endVertex = waypointIDs[i+1];
    BOOST_LOG_TRIVIAL(debug) << "Performing DFS from " << graph[beginVertex].getName();
    ParentMap parentMap;
    boost::depth_first_search(filteredGraph,
        boost::visitor(DfsVisitor(parentMap, true))
          .root_vertex(beginVertex));
    BOOST_LOG_TRIVIAL(debug) << "Determining all paths to " << graph[endVertex].getName();
    std::vector<VertexIDVec> paths;
    determineAllPaths(parentMap,
                      paths,
                      VertexIDVec(),
                      beginVertex,
                      endVertex);
    if (paths.empty()) {
      // No paths exist.
      return {};
    }
    intPaths.push_back(paths);
  }
  // Construct the final paths.
  auto paths = pathProduct(intPaths);
  for (auto &path : paths) {
    path.push_back(waypointIDs.back());
  }
  return paths;
}

/// Report a single path between a set of named points.
VertexIDVec Graph::getAnyPointToPoint(const VertexIDVec &waypointIDs,
                                      const VertexIDVec &avoidPointIDs) const {
  // Special case for paths between aliases of the same variable.
  if (isAliasPath(waypointIDs)) {
    BOOST_LOG_TRIVIAL(debug) << boost::format("%s is alias of %s")
                                  % graph[waypointIDs[0]].getName()
                                  % graph[waypointIDs[1]].getName();
    return {waypointIDs[0], waypointIDs[1]};
  }
  FilteredInternalGraph filteredGraph(graph,
                                      EdgePredicate(&graph),
                                      VertexPredicate(&avoidPointIDs));
  std::vector<VertexID> path;
  // Construct the path between each adjacent waypoint.
  for (std::size_t i = 0; i < waypointIDs.size()-1; ++i) {
    auto startVertex = waypointIDs[i];
    auto finishVertex = waypointIDs[i+1];
    BOOST_LOG_TRIVIAL(debug) << "Performing DFS from " << graph[startVertex].getName();
    ParentMap parentMap;
    boost::depth_first_search(filteredGraph,
        boost::visitor(DfsVisitor(parentMap, false))
          .root_vertex(startVertex));
    BOOST_LOG_TRIVIAL(debug) << "Determining a path to " << graph[finishVertex].getName();
    auto subPath = determinePath(parentMap,
                                 VertexIDVec(),
                                 startVertex,
                                 finishVertex);
    if (subPath.empty()) {
      // No path exists.
      return VertexIDVec();
    }
    std::reverse(std::begin(subPath), std::end(subPath));
    path.insert(std::end(path), std::begin(subPath), std::end(subPath)-1);
  }
  path.push_back(waypointIDs.back());
  return path;
}
