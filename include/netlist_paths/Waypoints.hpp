#ifndef NETLIST_PATHS_WAYPOINTS_HPP
#define NETLIST_PATHS_WAYPOINTS_HPP

#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Vertex.hpp"

namespace netlist_paths {

/// A class representing a set of waypoints to constrain a search for a path.
class Waypoints {
  std::vector<std::string> waypoints;
  std::vector<std::string> avoidPoints;
  bool gotStartPoint;
  bool gotFinishPoint;

public:

  /// Construct an empty Waypoints object.
  Waypoints() :
      gotStartPoint(false), gotFinishPoint(false) {}

  /// Construct a Waypoints object with patterns matching start and finish
  /// points.
  ///
  /// \param start    A pattern specifying a start point.
  /// \param finish   A pattern specifying an end point.
  Waypoints(const std::string start,
            const std::string end) :
      gotStartPoint(false), gotFinishPoint(false) {
    addStartPoint(start);
    addEndPoint(end);
  }

  /// Set a named start point.
  ///
  /// \param name A pattern specifying a start point.
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

  /// Set a named end point.
  ///
  /// \param name A pattern specifying an end point.
  void addEndPoint(const std::string name) {
    if (gotFinishPoint) {
      throw Exception("end point already defined");
    }
    gotFinishPoint = true;
    if (waypoints.size() > 0) {
      waypoints.insert(waypoints.end(), name);
    } else {
      waypoints.push_back(name);
    }
  }

  /// Add a through point.
  ///
  /// \param name A pattern specifying a mid point.
  void addThroughPoint(const std::string name) {
    if (waypoints.size() > 0) {
      waypoints.insert(waypoints.end()-(gotFinishPoint?1:0), name);
    } else {
      waypoints.push_back(name);
    }
  }

  /// Add a point to avoid.
  ///
  /// \param name A pattern specifying a mid point to avoid.
  void addAvoidPoint(const std::string name) {
    avoidPoints.push_back(name);
  }

  /// Access the waypoints.
  ///
  /// \returns An ordered vector of patterns for each waypoint.
  const std::vector<std::string> &getWaypoints() const { return waypoints; }

  /// Access the avoid points.
  ///
  /// \returns An ordered vector of patterns for each avoid point.
  const std::vector<std::string> &getAvoidPoints() const { return avoidPoints; }
};

} // End namespace.

#endif // NETLIST_PATHS_WAYPOINTS_HPP
