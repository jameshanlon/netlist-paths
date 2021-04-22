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

/// Wrapper for Python to manage the netlist object.
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
                     VertexNetlistType vertexType=VertexNetlistType::ANY) const;

  /// Lookup a single vertex of type register.
  VertexID getRegVertex(const std::string &name, bool matchAny) const;

  /// Lookup a single vertex of type register alias.
  VertexID getRegAliasVertex(const std::string &name, bool matchAny) const;

  /// Lookup a single vertex that is a startpoint.
  VertexID getStartVertex(const std::string &name, bool matchAny) const;

  /// Lookup a single vertex that is an end point.
  VertexID getEndVertex(const std::string &name, bool matchAny) const;

  /// Lookup a single vertex that is a mid point.
  VertexID getMidVertex(const std::string &name, bool matchAny) const;

  /// Lookup a DType by name.
  const std::shared_ptr<DType> getDType(const std::string &name) const;

  //===--------------------------------------------------------------------===//
  // Waypoints.
  //===--------------------------------------------------------------------===//

  /// Read a set of path waypoints to constrain a path query.
  VertexIDVec readWaypoints(Waypoints waypoints) const;

  /// Read a set of avoid points to constrain a path query.
  VertexIDVec readAvoidPoints(Waypoints waypoints) const;

public:
  Netlist() = delete;

  /// Construct a new netlist from an XML file.
  ///
  /// \param filename A path to the XML netlist file.
  Netlist(const std::string &filename);

  //===--------------------------------------------------------------------===//
  // Reporting of names and types.
  //===--------------------------------------------------------------------===//

  /// Lookup the data type string of a single vertex.
  ///
  /// \param name       A pattern specifying a name to match.
  /// \param vertexType The type of the vertex to lookup.
  ///
  /// \returns A string representing the data type.
  const std::string getVertexDTypeStr(const std::string &name,
                                      VertexNetlistType vertexType=VertexNetlistType::ANY) const;

  /// Lookup the data type width of a single vertex.
  ///
  /// \param name       A pattern specifying a name to match.
  /// \param vertexType The type of the vertex to lookup.
  ///
  /// \returns The width of the data type.
  size_t getVertexDTypeWidth(const std::string &name,
                             VertexNetlistType vertexType=VertexNetlistType::ANY) const;

  /// Lookup the width of a data type by name.
  ///
  /// \param name A pattern specifying a name to match.
  ///
  /// \returns The width of the data type.
  size_t getDTypeWidth(const std::string &name) const;

  //===--------------------------------------------------------------------===//
  // Basic path querying.
  //===--------------------------------------------------------------------===//

  /// Return true if a single startpoint matching a pattern exists.
  ///
  /// \param name A pattern specifying a name to match.
  ///
  /// \returns True if the startpoint exists.
  bool startpointExists(const std::string &name) const;

  /// Return true if a single endpoint matching a pattern 'name' exists.
  ///
  /// \param name A pattern specifying a name to match.
  ///
  /// \returns True if the endpoint exists.
  bool endpointExists(const std::string &name) const;

  /// Return true if any startpoint matching a pattern 'name' exists.
  ///
  /// \param name A pattern specifying a name to match.
  ///
  /// \returns True if any startpoint exists.
  bool anyStartpointExists(const std::string &name) const;

  /// Return true if any endpoint matching a pattern 'name' exists.
  ///
  /// \param name A pattern specifying a name to match.
  ///
  /// \returns True if any endpoint exists.
  bool anyEndpointExists(const std::string &name) const;

  /// Return true if a single register matching a pattern 'name' exists.
  ///
  /// \param name A pattern specifying a name to match.
  ///
  /// \returns True if the register exists.
  bool regExists(const std::string &name) const;

  /// Return true if any register matching a pattern 'name' exists.
  ///
  /// \param name A pattern specifying a name to match.
  ///
  /// \returns True if any register exists.
  bool anyRegExists(const std::string &name) const;

  /// Return a Boolean to indicate whether any path exists between two points.
  ///
  /// \param waypoints A waypoints object constraining the path.
  ///
  /// \returns True if a path exists.
  bool pathExists(Waypoints waypoints) const;

  /// Return any path between two points.
  ///
  /// \param waypoints A waypoints object constraining the path.
  ///
  /// \returns A path if one exists, otherwise an empty vector.
  std::vector<Vertex*> getAnyPath(Waypoints waypoints) const;

  /// Return all paths between two points, useful for testing.
  ///
  /// \param waypoints A waypoints object constraining the path.
  ///
  /// \returns All paths matching the waypoints, otherwise an empty vector.
  std::vector<std::vector<Vertex*> > getAllPaths(Waypoints waypoints) const;

  /// Return a vector of paths fanning out from a particular start point.
  ///
  /// \param startName A pattern matching a start point.
  ///
  /// \returns All paths fanning out from the matching startpoint, otherwise an empty vector.
  std::vector<std::vector<Vertex*> > getAllFanOut(const std::string startName) const;

  /// Return a vector of paths fanning out from a particular start point.
  ///
  /// \param endName A pattern matching an end point.
  ///
  /// \returns All paths fanning in to the matching endpoint, otherwise an empty vector.
  std::vector<std::vector<Vertex*> > getAllFanIn(const std::string endName) const;

  //===--------------------------------------------------------------------===//
  // Netlist access.
  //===--------------------------------------------------------------------===//

  /// Return a sorted list of unique named vertices in the netlist for searching.
  ///
  /// \param pattern A pattern to match vertices against.
  ///
  /// \returns A vector of Vertex references.
  std::vector<std::reference_wrapper<const Vertex>>
  getNamedVertices(const std::string pattern=std::string()) const;

  /// Return a vector of pointers to vertices that have names.
  std::vector<Vertex*> getNamedVerticesPtr(const std::string pattern=std::string()) const {
    return createVertexPtrVec(netlist.getVertices(pattern, VertexNetlistType::IS_NAMED));
  }

  /// Return a vector of pointers to net vertices.
  ///
  /// \param pattern A pattern to match vertices against.
  ///
  /// \returns A vector of pointers to Vertex objects.
  std::vector<Vertex*> getNetVerticesPtr(const std::string pattern=std::string()) const {
    return createVertexPtrVec(netlist.getVertices(pattern, VertexNetlistType::NET));
  }

  /// Return a vector of pointers to port vertices.
  ///
  /// \param pattern A pattern to match vertices against.
  ///
  /// \returns A vector of pointers to Vertex objects.
  std::vector<Vertex*> getPortVerticesPtr(const std::string pattern=std::string()) const {
    return createVertexPtrVec(netlist.getVertices(pattern, VertexNetlistType::PORT));
  }

  /// Return a vector of pointers to register vertices.
  ///
  /// \param pattern A pattern to match vertices against.
  ///
  /// \returns A vector of pointers to Vertex objects.
  std::vector<Vertex*> getRegVerticesPtr(const std::string pattern=std::string()) const {
    return createVertexPtrVec(netlist.getVertices(pattern, VertexNetlistType::REG));
  }

  /// Write a dot-file represenation of the netlist graph to a file.
  ///
  /// \param outputFilename The file to write the dot output to.
  void dumpDotFile(const std::string &outputFilename) const {
    netlist.dumpDotFile(outputFilename);
  }

  /// Return true if the netlist is empty.
  bool isEmpty() const { return netlist.numVertices() == 0; }
};

} // End namespace.

#endif // NETLIST_PATHS_NETLIST_HPP
