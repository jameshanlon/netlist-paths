#ifndef NETLIST_PATHS_PATH_HPP
#define NETLIST_PATHS_PATH_HPP

#include <vector>
#include <boost/log/trivial.hpp>
#include "netlist_paths/InternalGraph.hpp"
#include "netlist_paths/Options.hpp"

namespace netlist_paths {

/// A class to represent a path through the netlist graph.
class Path {

  std::vector<const Vertex*> vertices;

public:
  /// Construct an empty path.
  Path() {};

  /// Constrct a Path from a list of Vertices.
  ///
  /// \param sourceVertices A vector of vertices to copy.
  Path(const std::vector<const Vertex*> sourceVertices) {
    vertices.assign(sourceVertices.begin(), sourceVertices.end());
  }

  /// Copy constructor.
  ///
  /// \param path The Path to copy.
  Path(const Path &path) {
    vertices.assign(path.getVertices().begin(), path.getVertices().end());
  }

  /// Test equality with another path.
  ///
  /// \param other The Path to test against.
  ///
  /// \returns A Boolean indicating equal or not.
  bool operator==(const Path& other) const {
    return vertices == other.getVertices();
  }

  /// Return true if the vertex is contained in this path.
  ///
  /// \param vertex The Vertex to test.
  ///
  /// \returns A Boolean indicating whether vertex is contained in this path.
  bool contains(const Vertex *vertex) const {
    return std::find_if(vertices.begin(), vertices.end(),
                        [=](const Vertex *v) { return v->getID() == vertex->getID(); })
              != vertices.end();
  }

  /// Reverse the order of vertices in this path.
  void reverse() {
    std::reverse(std::begin(vertices), std::end(vertices));
  }

  /// Append a vertex to the end of this path.
  ///
  /// \param vertex The Vertex to append to this path.
  void appendVertex(const Vertex *vertex) { vertices.push_back(vertex); }

  /// Append a path to the end of this path.
  ///
  /// \param path The Path to append to this path.
  void appendPath(const Path &path) {
    vertices.insert(vertices.end(), path.getVertices().begin(), path.getVertices().end()-1);
  }

  // Various getters and setters.
  const std::vector<const Vertex*> &getVertices() const { return vertices; }
  const Vertex *getVertex(size_t index) const { return vertices[index]; }
  const Vertex *getStartVertex() const { return vertices.front(); }
  const Vertex *getFinishVertex() const { return vertices.back(); }
  size_t length() const { return vertices.size(); }
  bool empty() const { return vertices.empty(); }

  // Methods compatible with the boost::python wrappers by removing const from the Vertex* type.

  std::vector<Vertex*> getVerticesNoConst() const {
    std::vector<Vertex*> newVec;
    for (auto* vertex : vertices) {
      newVec.push_back(const_cast<Vertex*>(vertex));
    }
    return newVec;
  }
  Vertex *getVertexNoConst(size_t index) const { return const_cast<Vertex*>(vertices[index]); }
  Vertex *getStartVertexNoConst() const { return const_cast<Vertex*>(vertices.front()); }
  Vertex *getFinishVertexNoConst() const { return const_cast<Vertex*>(vertices.back()); }
};

} // End namespace.

#endif // NETLIST_PATHS_PATH_HPP
