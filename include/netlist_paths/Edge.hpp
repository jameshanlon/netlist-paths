#ifndef NETLIST_PATHS_EDGE_HPP
#define NETLIST_PATHS_EDGE_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include "netlist_paths/Location.hpp"
#include "netlist_paths/DTypes.hpp"

namespace netlist_paths {

/// A class representing an egde in the netlist graph.
class Edge {
  bool throughRegister;

public:
  Edge() : throughRegister(false) {};
  Edge(bool throughRegister) : throughRegister(throughRegister) {}

  void setThroughRegister(bool value) { throughRegister = value; }
  bool isThroughRegister() const { return throughRegister; }
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_EDGE_HPP
