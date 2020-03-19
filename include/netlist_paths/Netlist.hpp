#ifndef NETLIST_PATHS_NETLIST_HPP
#define NETLIST_PATHS_NETLIST_HPP

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

class Netlist {
private:
  Graph graph;
  boost::dynamic_properties dp;
  std::string topName;
  std::vector<VertexDesc> waypoints;

  bool vertexCompare(const VertexDesc a, const VertexDesc b) const;
  bool vertexEqual(const VertexDesc a, const VertexDesc b) const;

  VertexDesc getVertexDesc(const std::string &name,
                           bool matchVertex (const VertexProperties &p)) const;
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
  Netlist();
  bool parseGraphViz(std::istream &in);
  void parseFile(const std::string &filename);
  void mergeDuplicateVertices();
  void checkGraph() const;
  void dumpDotFile(const std::string &outputFilename) const;
  int maxNameLength(const Path &path) const;
  std::vector<VertexDesc> getNames() const;
  void printNames(std::vector<VertexDesc> &names) const;
  void printPathReport(const Path &path) const;
  void printPathReport(const std::vector<Path> &paths) const;
  VertexDesc getStartVertex(const std::string &name) const {
    return getVertexDesc(name, isStartPoint);
  }
  VertexDesc getEndVertex(const std::string &name) const {
    return getVertexDesc(name, isEndPoint);
  }
  VertexDesc getMidVertex(const std::string &name) const {
    return getVertexDesc(name, isMidPoint);
  }
  VertexDesc getRegVertex(const std::string &name) const {
    return getVertexDesc(name, isReg);
  }
  VertexDesc getStartVertexExcept(const std::string &name) const;
  VertexDesc getEndVertexExcept(const std::string &name) const;
  VertexDesc getMidVertexExcept(const std::string &name) const;
  std::vector<Path> getAllFanOut(VertexDesc startVertex) const;
  std::vector<Path> getAllFanOut(const std::string &startName) const;
  std::vector<Path> getAllFanIn(VertexDesc endVertex) const;
  std::vector<Path> getAllFanIn(const std::string &endName) const;
  unsigned getfanOutDegree(VertexDesc startVertex);
  unsigned getfanOutDegree(const std::string &startName);
  unsigned getFanInDegree(VertexDesc endVertex);
  unsigned getFanInDegree(const std::string &endName);
  Path getAnyPointToPoint() const;
  std::vector<Path> getAllPointToPoint() const;
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
  void clearWaypoints() { waypoints.clear(); }
  const std::string &getVertexName(VertexDesc vertex) const { return graph[vertex].name; }
  bool startpointExists(const std::string &name) const noexcept {
    return getStartVertex(name) != boost::graph_traits<Graph>::null_vertex();
  }
  bool endpointExists(const std::string &name) const noexcept {
    return getEndVertex(name) != boost::graph_traits<Graph>::null_vertex();
  }
  bool regExists(const std::string &name) const noexcept {
    return getRegVertex(name) != boost::graph_traits<Graph>::null_vertex();
  }
  bool pathExists(const std::string &start, const std::string &end);
};

} // End namespace.

#endif // NETLIST_PATHS_NETLIST_HPP
