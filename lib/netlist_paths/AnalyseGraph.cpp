#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <boost/filesystem.hpp>
#include <boost/graph/reverse_graph.hpp>
#include "netlist_paths/AnalyseGraph.hpp"
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Graph.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/utilities.hpp"

namespace fs = boost::filesystem;
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
      typename boost::graph_traits<Graph>::vertex_descriptor src, dst;
      src = boost::source(edge, graph);
      dst = boost::target(edge, graph);
      parentMap[dst].push_back(src);
    }
    return;
  }
  // Visit all edges of a all vertices.
  template<typename Edge, typename Graph>
  void examine_edge(Edge edge, const Graph &graph) const {
    if (allPaths) {
      typename boost::graph_traits<Graph>::vertex_descriptor src, dst;
      src = boost::source(edge, graph);
      dst = boost::target(edge, graph);
      parentMap[dst].push_back(src);
    }
    return;
  }
};

VertexType AnalyseGraph::getVertexType(const std::string &type) const {
       if (type == "LOGIC")   return VertexType::LOGIC;
  else if (type == "ASSIGN")  return VertexType::ASSIGN;
  else if (type == "ASSIGNW") return VertexType::ASSIGNW;
  else if (type == "ALWAYS")  return VertexType::ALWAYS;
  else if (type == "INITIAL") return VertexType::INITIAL;
  else if (type == "REG_SRC") return VertexType::REG_SRC;
  else if (type == "REG_DST") return VertexType::REG_DST;
  else if (type == "VAR")     return VertexType::VAR;
  else if (type == "WIRE")    return VertexType::WIRE;
  else if (type == "PORT")    return VertexType::PORT;
  else {
    throw netlist_paths::Exception(std::string("unexpected vertex type: ")+type);
  }
}

const char *AnalyseGraph::getVertexTypeStr(VertexType type) const {
  switch (type) {
    case VertexType::LOGIC:   return "LOGIC";
    case VertexType::ASSIGN:  return "ASSIGN";
    case VertexType::ASSIGNW: return "ASSIGNW";
    case VertexType::ALWAYS:  return "ALWAYS";
    case VertexType::INITIAL: return "INITIAL";
    case VertexType::REG_SRC: return "REG_SRC";
    case VertexType::REG_DST: return "REG_DST";
    case VertexType::VAR:     return "VAR";
    case VertexType::WIRE:    return "WIRE";
    case VertexType::PORT:    return "PORT";
    default:                  return "UNKNOWN";
  }
}

VertexDirection AnalyseGraph::
getVertexDirection(const std::string &direction) const {
       if (direction == "NONE")   return VertexDirection::NONE;
  else if (direction == "INPUT")  return VertexDirection::INPUT;
  else if (direction == "OUTPUT") return VertexDirection::OUTPUT;
  else if (direction == "INOUT")  return VertexDirection::INOUT;
  else {
    throw netlist_paths::Exception(std::string("unexpected vertex direction: ")
                                     +direction);
  }
}

const char *AnalyseGraph::
getVertexDirectionStr(VertexDirection direction) const {
  switch (direction) {
    case VertexDirection::NONE:   return "NONE";
    case VertexDirection::INPUT:  return "INPUT";
    case VertexDirection::OUTPUT: return "OUTPUT";
    case VertexDirection::INOUT:  return "INOUT";
    default:                      return "UNKNOWN";
  }
}

/// Parse a graph input file and return a list of Vertices and a list of Edges.
void AnalyseGraph::parseFile(const std::string &filename) {
  DEBUG(std::cout << "Parsing input file\n");
  std::fstream infile(filename);
  std::string line;
  if (!infile.is_open()) {
    throw Exception("could not open file");
  }
  while (std::getline(infile, line)) {
    std::vector<std::string> tokens;
    boost::split(tokens, line, boost::is_any_of(" "));
    // "VERTEX" <id> <type> <location>
    // "VERTEX" <id> <type> <direction> <name> <location>
    if (tokens[0] == "VERTEX") {
      int id = std::atoi(tokens[1].c_str());
      auto type = getVertexType(tokens[2]);
      if (id == 0) {
        throw Exception("vertex has NULL ID");
      }
      if (isVertexTypeLogic(type)) {
        // Unnamed logic vertex.
        auto &loc = tokens[3];
        vertices.push_back(Vertex(id, type, loc));
      } else {
        // Named net/port/register.
        auto direction = getVertexDirection(tokens[3]);
        auto &name = tokens[4];
        auto &loc = tokens[5];
        vertices.push_back(Vertex(id, type, direction, name, loc));
      }
    // "EDGE" <vertex-src-id> "->" <vertex-dst-id>
    } else if (tokens[0] == "EDGE") {
      auto edge = Edge(std::stoi(tokens[1]),
                       std::stoi(tokens[3]));
      edges.push_back(edge);
    } else {
      throw Exception(std::string("unexpected line: ")+line);
    }
  }
  // Build and check the graph.
  buildGraph();
  checkGraph();
}

/// Build a Boost graph object.
void AnalyseGraph::buildGraph() {
  // Construct graph.
  DEBUG(std::cout << "Constructing graph\n");
  graph = std::make_unique<Graph>(vertices.size());
  for (auto &edge : edges) {
    if (!boost::edge(edge.src, edge.dst, *graph).second) {
      boost::add_edge(edge.src, edge.dst, *graph);
    }
  }
}

void AnalyseGraph::checkGraph() {
  // Perform some checks.
  DEBUG(std::cout << "Checking graph\n");
  for (auto &vertex : vertices) {
    // Source registers don't have in edges.
    if (vertex.type == VertexType::REG_SRC) {
      if (boost::in_degree(boost::vertex(vertex.id, *graph), *graph) > 0)
         std::cout << "Warning: source reg " << vertex.name
                   << " (" << vertex.id << ") has in edges" << "\n";
    }
    // Destination registers don't have out edges.
    if (vertex.type == VertexType::REG_DST) {
      if (boost::out_degree(boost::vertex(vertex.id, *graph), *graph) > 0)
        std::cout << "Warning: destination reg " << vertex.name
                  << " (" << vertex.id << ") has out edges"<<"\n";
    }
    // NOTE: vertices may be incorrectly marked as reg if a field of a
    // structure has a delayed assignment to a field of it.
  }
}

int AnalyseGraph::getVertexId(const std::string &name,
                              VertexType type) const {
  auto pred = [&] (const Vertex &v) { return v.type == type &&
                                             v.name == name; };
  auto it = std::find_if(std::begin(vertices), std::end(vertices), pred);
  if (it != std::end(vertices)) {
    DEBUG(std::cout<<"Vertex "<<it->id<<" matches "<<name
                   <<" of type "<<getVertexTypeStr(type)<<"\n");
    return it->id;
  }
  return NULL_VERTEX_ID;
}

int AnalyseGraph::getStartVertexId(const std::string &name) const {
  if (int v = getVertexId(name, VertexType::REG_SRC)) return v;
  if (int v = getVertexId(name, VertexType::VAR))     return v;
  if (int v = getVertexId(name, VertexType::WIRE))    return v;
  if (int v = getVertexId(name, VertexType::PORT))    return v;
  throw Exception(std::string("could not find start vertex ")+name);
}

int AnalyseGraph::getEndVertexId(const std::string &name) const {
  if (int v = getVertexId(name, VertexType::REG_DST)) return v;
  if (int v = getVertexId(name, VertexType::VAR))     return v;
  if (int v = getVertexId(name, VertexType::WIRE))    return v;
  if (int v = getVertexId(name, VertexType::PORT))    return v;
  throw Exception(std::string("could not find end vertex ")+name);
}

int AnalyseGraph::getMidVertexId(const std::string &name) const {
  if (int v = getVertexId(name, VertexType::VAR))  return v;
  if (int v = getVertexId(name, VertexType::WIRE)) return v;
  if (int v = getVertexId(name, VertexType::PORT)) return v;
  throw Exception(std::string("could not find mid vertex ")+name);
}

void AnalyseGraph::dumpPath(const std::vector<int> &path) const {
  for (int vertexId : path) {
    if (!vertices[vertexId-1].isLogic()) {
      std::cout << "  " << vertices[vertexId-1].name << "\n";
    }
  }
}

/// Given the tree structure from a DFS, traverse the tree from leaf to root to
/// return a path.
std::vector<int> AnalyseGraph::determinePath(ParentMap &parentMap,
                                             std::vector<int> path,
                                             int startVertexId,
                                             int endVertexId) const {
  path.push_back(endVertexId);
  if (endVertexId == startVertexId) {
    return path;
  }
  if (parentMap[endVertexId].size() == 0)
    return std::vector<int>();
  assert(parentMap[endVertexId].size() == 1);
  int nextVertexId = parentMap[endVertexId].front();
  assert(std::find(std::begin(path),
                   std::end(path),
                   nextVertexId) == std::end(path));
  return determinePath(parentMap, path, startVertexId, nextVertexId);
}

/// Determine all paths between a start and an end point.
/// This performs a DFS starting at the end point. It is not feasible for large
/// graphs since the number of simple paths grows exponentially.
void AnalyseGraph::determineAllPaths(ParentMap &parentMap,
                                     std::vector<std::vector<int>> &result,
                                     std::vector<int> path,
                                     int startVertexId,
                                     int endVertexId) const {
  path.push_back(endVertexId);
  if (endVertexId == startVertexId) {
    DEBUG(std::cout << "FOUND PATH\n");
    result.push_back(path);
    return;
  }
  DEBUG(std::cout<<"length "<<path.size()<<" vertex "<<endVertexId<<"\n");
  DEBUG(dumpPath(path));
  DEBUG(std::cout<<(parentMap[endVertexId].empty()?"DEAD END\n":""));
  for (auto &vertex : parentMap[endVertexId]) {
    if (std::find(std::begin(path), std::end(path), vertex) == std::end(path)) {
      determineAllPaths(parentMap, result, path, startVertexId, vertex);
    } else {
      DEBUG(std::cout << "CYCLE DETECTED\n");
    }
  }
}

/// Dump a Graphviz dotfile of the netlist graph for visualisation.
void AnalyseGraph::dumpDotFile(const std::string &outputFilename) const {
  std::stringstream ss;
  ss << "digraph netlist_graph {\n";
  for (auto &vertex : vertices)
    ss << "  " << vertex.id << " [label=\""
       << getVertexTypeStr(vertex.type) << "\\n"
       << vertex.name << " (" << vertex.id << ")\"];\n";
  for (auto &edge : edges)
    ss << "  " << edge.src << " -> " << edge.dst << ";\n";
  ss << "}\n";
  // Write to file.
  std::ofstream outputFile(outputFilename);
  if (!outputFile.is_open())
    throw Exception(std::string("unable to open ")+outputFilename);
  outputFile << ss.str();
  outputFile.close();
  // Print command line to generate graph file.
  DEBUG(std::cout << "dot -Tpdf " << outputFilename << " -o graph.pdf\n");
}

/// Dump unique names of vars/regs/wires in the netlist for searching.
void AnalyseGraph::dumpVertexNames() const {
  std::unordered_set<std::string> names;
  for (auto &vertex : vertices) {
    if (vertex.isLogic())
      continue;
    names.insert(std::string(getVertexTypeStr(vertex.type))+" "+vertex.name);
  }
  std::vector<std::string> sortedNames(names.begin(), names.end());
  std::sort(sortedNames.begin(), sortedNames.end());
  for (auto &name : sortedNames) {
    std::cout << std::setw(8) << name << "\n";
  }
}

/// Pretty print a path (some sequence of vertices).
void AnalyseGraph::printPathReport(const std::vector<int> &path) const {
  // Determine the max length of a name.
  size_t maxNameLength = 0;
  for (auto &vertexId : path) {
    if (vertices[vertexId-1].canIgnore())
      continue;
    maxNameLength = std::max(maxNameLength, vertices[vertexId-1].name.size());
  }
  // Print each vertex on the path.
  for (auto it = path.begin(); it != path.end(); ++it) {
    auto &vertex = vertices[*it-1];
    if (vertex.canIgnore())
      continue;
    auto srcPath = options.filenamesOnly ? fs::path(vertex.loc).filename()
                                         : fs::path(vertex.loc);
    if (options.netsOnly) {
      if (!vertex.isLogic()) {
        std::cout << "  " << std::left
                  << std::setw(maxNameLength+1) << vertex.name
                  << srcPath.string() << "\n";
      }
    } else {
      if (vertex.isLogic()) {
        std::cout << "  " << std::left
                  << std::setw(maxNameLength+1)
                  << getVertexTypeStr(vertex.type)
                  << std::setw(VERTEX_TYPE_STR_MAX_LEN)
                  << "LOGIC"
                  << srcPath.string() << "\n";
      } else {
        std::cout << "  " << std::left
                  << std::setw(maxNameLength+1)
                  << vertex.name
                  << std::setw(VERTEX_TYPE_STR_MAX_LEN)
                  << getVertexTypeStr(vertex.type)
                  << srcPath.string() << "\n";
      }
    }
  }
}

/// Report all paths fanning out from a net/register/port.
void AnalyseGraph::reportAllFanout(const std::string &startName) const {
  int startVertexId = getStartVertexId(startName);
  DEBUG(std::cout << "Performing DFS from "
                  << vertices[startVertexId].name << "\n");
  ParentMap parentMap;
  boost::depth_first_search(*graph,
      boost::visitor(DfsVisitor(parentMap, false))
        .root_vertex(startVertexId));
  // Check for a path between startPoint and each register.
  int pathCount = 0;
  for (auto &vertex : vertices) {
    if (vertex.isEndPoint()) {
      auto path = determinePath(parentMap, std::vector<int>(),
                                startVertexId, vertex.id);
      std::reverse(std::begin(path), std::end(path));
      if (!path.empty()) {
        std::cout << "Path " << ++pathCount << "\n";
        printPathReport(path);
        std::cout << "\n";
      }
    }
  }
  std::cout << "Found " << pathCount << " path(s)\n";
}

/// Report all paths fanning into a net/register/port.
void AnalyseGraph::reportAllFanin(const std::string &endName) const {
  auto reverseGraph = boost::make_reverse_graph(*graph);
  int endVertexId = getEndVertexId(endName);
  DEBUG(std::cout << "Performing DFS in reverse graph from "
                  << vertices[endVertexId].name << "\n");
  ParentMap parentMap;
  boost::depth_first_search(reverseGraph,
      boost::visitor(DfsVisitor(parentMap, false))
        .root_vertex(endVertexId));
  // Check for a path between endPoint and each register.
  int pathCount = 0;
  for (auto &vertex : vertices) {
    if (vertex.isStartPoint()) {
      auto path = determinePath(parentMap, std::vector<int>(),
                                endVertexId, vertex.id);
      if (!path.empty()) {
        std::cout << "Path " << ++pathCount << "\n";
        printPathReport(path);
        std::cout << "\n";
      }
    }
  }
  std::cout << "Found " << pathCount << " paths\n";
}

/// Report a single path between a set of named points.
void AnalyseGraph::
reportAnyPointToPoint(const std::vector<int> &waypoints) const {
  std::vector<int> path;
  // Construct the path between each adjacent waypoints.
  for (size_t i = 0; i < waypoints.size()-1; ++i) {
    int startVertexId = waypoints[i];
    int endVertexId = waypoints[i+1];
    DEBUG(std::cout << "Performing DFS from "
                    << vertices[startVertexId].name << "\n");
    ParentMap parentMap;
    boost::depth_first_search(*graph,
        boost::visitor(DfsVisitor(parentMap, false))
          .root_vertex(startVertexId));
    DEBUG(std::cout << "Determining a path to " << endVertexId << "\n");
    auto subPath = determinePath(parentMap, std::vector<int>(),
                                 startVertexId, endVertexId);
    if (subPath.empty())
        throw Exception(std::string("no path from ")
                            +vertices[startVertexId].name+" to "
                            +vertices[endVertexId].name);
    std::reverse(std::begin(subPath), std::end(subPath));
    path.insert(std::end(path), std::begin(subPath), std::end(subPath)-1);
  }
  path.push_back(waypoints.back());
  printPathReport(path);
}

/// Report all paths between start and end points.
void AnalyseGraph::
reportAllPointToPoint(const std::vector<int> &waypoints) const {
  if (waypoints.size() > 2)
    throw Exception("through points not supported for all paths");
  DEBUG(std::cout << "Performing DFS\n");
  ParentMap parentMap;
  boost::depth_first_search(*graph,
      boost::visitor(DfsVisitor(parentMap, true))
        .root_vertex(waypoints[0]));
  DEBUG(std::cout << "Determining all paths\n");
  std::vector<std::vector<int>> paths;
  determineAllPaths(parentMap, paths, std::vector<int>(),
                    waypoints[0], waypoints[1]);
  int count = 0;
  for (auto &path : paths) {
    std::reverse(std::begin(path), std::end(path));
    std::cout << "PATH " << ++count << ":\n";
    printPathReport(path);
  }
}
