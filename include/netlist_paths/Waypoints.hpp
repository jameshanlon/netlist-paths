#ifndef NETLIST_PATHS_WAYPOINTS_HPP
#define NETLIST_PATHS_WAYPOINTS_HPP

#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

class Waypoints {
  std::vector<std::string> waypoints;
  std::vector<std::string> avoidPoints;
  bool gotStartPoint;
  bool gotFinishPoint;
  bool matchAnyStartPoint;
  bool matchAnyFinishPoint;

public:
  Waypoints() :
      gotStartPoint(false), gotFinishPoint(false),
      matchAnyStartPoint(false), matchAnyFinishPoint(false) {}

  Waypoints(const std::string start,
            const std::string finish,
            bool matchAny=false) :
      gotStartPoint(false), gotFinishPoint(false),
      matchAnyStartPoint(matchAny), matchAnyFinishPoint(matchAny) {
    addStartPoint(start);
    addFinishPoint(finish);
  }

  void addStartPoint(const std::string name) {
    if (gotStartPoint) {
      throw Exception("start point already defined");
    }
    gotStartPoint = true;
    if (waypoints.size() > 0) {
      waypoints.insert(waypoints.begin(), name);
    } else {
      waypoints.push_back(name);
    }
  }

  void addFinishPoint(const std::string name) {
    if (gotFinishPoint) {
      throw Exception("finish point already defined");
    }
    gotFinishPoint = true;
    if (waypoints.size() > 0) {
      waypoints.insert(waypoints.end(), name);
    } else {
      waypoints.push_back(name);
    }
  }

  void addAnyStartPoint(const std::string name) {
    addStartPoint(name);
    matchAnyStartPoint = true;
  }

  void addAnyFinishPoint(const std::string name) {
    addFinishPoint(name);
    matchAnyFinishPoint = true;
  }

  void addThroughPoint(const std::string name) {
    if (waypoints.size() > 0) {
      waypoints.insert(waypoints.end()-(gotFinishPoint?1:0), name);
    } else {
      waypoints.push_back(name);
    }
  }

  void addAvoidPoint(const std::string name) {
    avoidPoints.push_back(name);
  }

  const std::vector<std::string> &getWaypoints() const { return waypoints; }
  const std::vector<std::string> &getAvoidPoints() const { return avoidPoints; }
  bool anyStart() const { return matchAnyStartPoint; }
  bool anyFinish() const { return matchAnyFinishPoint; }
};

}; // End namespace.

#endif // NETLIST_PATHS_WAYPOINTS_HPP
