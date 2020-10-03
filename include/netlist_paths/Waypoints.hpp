#ifndef NETLIST_PATHS_WAYPOINTS_HPP
#define NETLIST_PATHS_WAYPOINTS_HPP

#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

class Waypoints {
  std::vector<std::string> waypoints;

public:
  Waypoints() {}
  Waypoints(const std::string start,
            const std::string finish) {
    waypoints.push_back(start);
    waypoints.push_back(finish);
  }

  void add(const std::string name) {
    waypoints.push_back(name);
  }

  std::vector<std::string>::iterator begin() { return waypoints.begin(); }
  std::vector<std::string>::iterator end() { return waypoints.end(); }
  bool empty() const { return waypoints.empty(); }
  size_t size() const { return waypoints.size(); }
};

}; // End namespace.

#endif // NETLIST_PATHS_WAYPOINTS_HPP
