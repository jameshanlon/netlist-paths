#ifndef NETLIST_PATHS_ANALYSE_GRAPH_HPP
#define NETLIST_PATHS_ANALYSE_GRAPH_HPP

#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include "netlist_paths/Graph.hpp"

namespace netlist_paths {

using ParentMap = std::map<int, std::vector<int>>;

class AnalyseGraph {
private:
  using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
                                      boost::bidirectionalS>;
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
  std::unique_ptr<Graph> graph;

  void buildGraph();
  void checkGraph();
  VertexType getVertexType(const std::string &type) const;
  const char *getVertexTypeStr(VertexType type) const;
  int getVertexId(const std::string &name, VertexType type) const;
  void dumpPath(const std::vector<int> &path) const;
  std::vector<int> determinePath(ParentMap &parentMap,
                                 std::vector<int> path,
                                 int startVertexId,
                                 int endVertexId) const;
  void determineAllPaths(ParentMap &parentMap,
                         std::vector<std::vector<int>> &result,
                         std::vector<int> path,
                         int startVertexId,
                         int endVertexId) const;
  void printPathReport(const std::vector<int> &path) const;

public:
  AnalyseGraph() {}
  void parseFile(const std::string &filename);
  void dumpDotFile(const std::string &outputFilename) const;
  void dumpVertexNames() const;
  int getStartVertexId(const std::string &name) const;
  int getEndVertexId(const std::string &name) const;
  int getMidVertexId(const std::string &name) const;
  void reportAllFanout(const std::string &startName) const;
  void reportAllFanin(const std::string &endName) const;
  void reportAnyPointToPoint(const std::vector<int> &waypoints) const;
  void reportAllPointToPoint(const std::vector<int> &waypoints) const;
  std::size_t getNumVertices() const { return vertices.size(); }
  std::size_t getNumEdges() const { return edges.size(); }
};

} // End namespace.

#endif // NETLIST_PATHS_ANALYSE_GRAPH_HPP

