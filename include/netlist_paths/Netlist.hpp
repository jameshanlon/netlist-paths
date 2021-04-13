#ifndef NETLIST_PATHS_NETLIST_HPP
#define NETLIST_PATHS_NETLIST_HPP

#include <memory>
#include <iostream>
#include <ostream>
#include <sstream>
#include <boost/format.hpp>
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Graph.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/Waypoints.hpp"

namespace netlist_paths {

/// Wrapper for Python to manage the netlist paths object.
class Netlist {
  Graph netlist;
  std::vector<File> files;
  std::vector<std::shared_ptr<DType>> dtypes;
  std::vector<VertexID> waypoints;

  //===--------------------------------------------------------------------===//
  // Utility functions.
  //===--------------------------------------------------------------------===//

  std::vector<VertexID> getNamedVertexIds(const std::string &regex="") const;

  std::vector<Vertex*> createVertexPtrVec(VertexIDVec vertices) const;

  std::vector<std::vector<Vertex*> >
  createVertexPtrVecVec(std::vector<VertexIDVec> paths) const;

  //===--------------------------------------------------------------------===//
  // Vertex lookup, with error reporting.
  //===--------------------------------------------------------------------===//

  const std::string reportMultipleMatches(VertexIDVec vertices,
                                          const std::string name,
                                          const std::string patternType="") const;

  /// Lookup a single vertex and report an error if there are multiple matches.
  VertexID getVertex(const std::string &name,
                     VertexGraphType vertexType=VertexGraphType::ANY) const;

  /// Lookup a single vertex of type register.
  VertexID getRegVertex(const std::string &name) const;

  /// Lookup a single vertex that is a startpoint.
  VertexID getStartVertex(const std::string &name) const;

  /// Lookup a single vertex that is an end point.
  VertexID getEndVertex(const std::string &name) const;

  /// Lookup a single vertex that is a mid point.
  VertexID getMidVertex(const std::string &name) const;

  /// Lookup a DType by name.
  const std::shared_ptr<DType> getDType(const std::string &name) const;

public:
  Netlist() = delete;
  Netlist(const std::string &filename);

  //===--------------------------------------------------------------------===//
  // Reporting of names and types.
  //===--------------------------------------------------------------------===//

  /// Lookup the data type string of a single vertex.
  const std::string getVertexDTypeStr(const std::string &name,
                                      VertexGraphType vertexType=VertexGraphType::ANY) const;

  /// Lookup the data type width of a single vertex.
  size_t getVertexDTypeWidth(const std::string &name,
                             VertexGraphType vertexType=VertexGraphType::ANY) const;

  /// Lookup a DType's width by name.
  size_t getDTypeWidth(const std::string &name) const;

  //===--------------------------------------------------------------------===//
  // Waypoints.
  //===--------------------------------------------------------------------===//

  VertexIDVec readWaypoints(Waypoints waypoints) const;
  VertexIDVec readAvoidPoints(Waypoints waypoints) const;

  //===--------------------------------------------------------------------===//
  // Basic path querying.
  //===--------------------------------------------------------------------===//

  bool startpointExists(const std::string &name) const;

  bool endpointExists(const std::string &name) const;

  bool anyStartpointExists(const std::string &name) const;

  bool anyEndpointExists(const std::string &name) const;

  bool anyRegExists(const std::string &name) const;

  bool regExists(const std::string &name) const;

  /// Return a Boolean to indicate whether any path exists between two points.
  bool pathExists(Waypoints waypoints) const;

  /// Return any path between two points.
  std::vector<Vertex*> getAnyPath(Waypoints waypoints) const;

  /// Return all paths between two points, useful for testing.
  std::vector<std::vector<Vertex*> > getAllPaths(Waypoints waypoints) const;

  /// Return a vector of paths fanning out from a particular start point.
  std::vector<std::vector<Vertex*> > getAllFanOut(const std::string startName) const;

  /// Return a vector of paths fanning out from a particular start point.
  std::vector<std::vector<Vertex*> > getAllFanIn(const std::string endName) const;

  //===--------------------------------------------------------------------===//
  // Netlist access.
  //===--------------------------------------------------------------------===//

  /// Return a sorted list of unique named vertices in the netlist for searching.
  std::vector<std::reference_wrapper<const Vertex>>
  getNamedVertices(const std::string &pattern="") const;

  /// Return a vector of pointers to vertices that have names.
  std::vector<Vertex*> getNamedVerticesPtr(const std::string pattern=std::string()) const {
    return createVertexPtrVec(netlist.getVertices(pattern, VertexGraphType::IS_NAMED));
  }

  /// Return a vector of pointers to net vertices.
  std::vector<Vertex*> getNetVerticesPtr(const std::string pattern=std::string()) const {
    return createVertexPtrVec(netlist.getVertices(pattern, VertexGraphType::NET));
  }

  /// Return a vector of pointers to port vertices.
  std::vector<Vertex*> getPortVerticesPtr(const std::string pattern=std::string()) const {
    return createVertexPtrVec(netlist.getVertices(pattern, VertexGraphType::PORT));
  }

  /// Return a vector of pointers to register vertices.
  std::vector<Vertex*> getRegVerticesPtr(const std::string pattern=std::string()) const {
    return createVertexPtrVec(netlist.getVertices(pattern, VertexGraphType::REG));
  }

  void dumpDotFile(const std::string &outputFilename) const {
    netlist.dumpDotFile(outputFilename);
  }

  bool isEmpty() const { return netlist.numVertices() == 0; }
};

} // End namespace.

#endif // NETLIST_PATHS_NETLIST_HPP
