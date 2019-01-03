#ifndef NETLIST_PATHS_ANALYSE_GRAPH_HPP
#define NETLIST_PATHS_ANALYSE_GRAPH_HPP

#include <string>
#include <vector>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

using Graph = boost::adjacency_list<boost::vecS,
                                    boost::vecS,
                                    boost::bidirectionalS,
                                    VertexProperties>;
using VertexDesc = boost::graph_traits<Graph>::vertex_descriptor;
using ParentMap = std::map<VertexDesc, std::vector<VertexDesc>>;
using Path = std::vector<VertexDesc>;

class AnalyseGraph {
private:
  Graph graph;
  boost::dynamic_properties dp;
  std::vector<VertexDesc> waypoints;

  VertexDesc getVertexDesc(const std::string &name, VertexType type) const;
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
  AnalyseGraph();
  void parseFile(const std::string &filename);
  void dumpDotFile(const std::string &outputFilename) const;
  void dumpVertexNames() const;
  void printPathReport(const Path &path) const;
  void printPathReport(const std::vector<Path> &paths) const;
  void printFanOuts(const std::vector<std::pair<VertexDesc, size_t>> &fanOuts,
                    size_t min=2) const;
  VertexDesc getVertex(const std::string &name,
                       const std::vector<VertexType> &types) const;
  VertexDesc getStartVertex(const std::string &name) const;
  VertexDesc getEndVertex(const std::string &name) const;
  VertexDesc getMidVertex(const std::string &name) const;
  std::vector<Path> getAllFanOut(VertexDesc startVertex) const;
  std::vector<Path> getAllFanOut(const std::string &startName) const;
  std::vector<Path> getAllFanIn(VertexDesc endVertex) const;
  std::vector<Path> getAllFanIn(const std::string &endName) const;
  Path getAnyPointToPoint() const;
  std::vector<Path> getAllPointToPoint() const;
  std::vector<std::pair<VertexDesc, size_t>> getAllFanOutDegrees() const;
  std::size_t getNumVertices() const { return boost::num_vertices(graph); }
  std::size_t getNumEdges() const { return boost::num_edges(graph); }
  void addStartpoint(const std::string &name) {
    waypoints.push_back(getStartVertex(name));
  }
  void addEndpoint(const std::string &name) {
    waypoints.push_back(getEndVertex(name));
  }
  void addWaypoint(const std::string &name) {
    waypoints.push_back(getMidVertex(name));
  }
};

} // End namespace.

#endif // NETLIST_PATHS_ANALYSE_GRAPH_HPP
