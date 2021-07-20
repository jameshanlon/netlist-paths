#ifndef NETLIST_PATHS_INTERNAL_GRAPH_HPP
#define NETLIST_PATHS_INTERNAL_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include "netlist_paths/Vertex.hpp"
#include "netlist_paths/Edge.hpp"

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

}; // End namespace.

#endif // NETLIST_PATHS_INTERNAL_GRAPH_HPP
