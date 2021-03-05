#include <regex>
#include <boost/format.hpp>
#include "netlist_paths/Netlist.hpp"

using namespace netlist_paths;

std::vector<std::reference_wrapper<const Vertex>>
Netlist::getNamedVertices(const std::string &pattern) const {
  // Collect vertices.
  std::vector<std::reference_wrapper<const Vertex>> vertices;
  for (auto vertexId : netlist.getVertices(pattern, VertexGraphType::IS_NAMED)) {
    vertices.push_back(std::ref(netlist.getVertex(vertexId)));
  }
  // Sort them.
  auto compare = [](const Vertex& a, const Vertex& b) {
                   return a.compareLessThan(b); };
  std::sort(vertices.begin(), vertices.end(), compare);
  return vertices;
}

VertexIDVec Netlist::readWaypoints(Waypoints waypoints) const {
  VertexIDVec waypointIDs;
  for (auto it = waypoints.getWaypoints().begin();
            it != waypoints.getWaypoints().end(); ++it) {
    VertexID vertex;
    // Start
    if (it == waypoints.getWaypoints().begin()) {
      vertex = waypoints.anyStart() ? getAnyBeginVertex(*it) : getBeginVertex(*it);
      if (vertex == netlist.nullVertex()) {
        throw Exception(std::string("could not find start vertex matching ")+*it);
      }
    // Finish
    } else if (it+1 == waypoints.getWaypoints().end()) {
      vertex = waypoints.anyFinish() ? getAnyEndVertex(*it) : getEndVertex(*it);
      if (vertex == netlist.nullVertex()) {
        throw Exception(std::string("could not find end vertex matching ")+*it);
      }
    // Mid
    } else {
      vertex = getMidVertex(*it);
      if (vertex == netlist.nullVertex()) {
        throw Exception(std::string("could not find through vertex ")+*it);
      }
    }
    waypointIDs.push_back(vertex);
  }
  return waypointIDs;
}

VertexIDVec Netlist::readAvoidPoints(Waypoints waypoints) const {
  VertexIDVec avoidPointIDs;
  for (auto name : waypoints.getAvoidPoints()) {
    auto vertex = getMidVertex(name);
    if (vertex == netlist.nullVertex()) {
      throw Exception(std::string("could not find vertex to avoid ")+name);
    }
    avoidPointIDs.push_back(vertex);
  }
  // Sort the IDs so they can be binary searched.
  std::sort(avoidPointIDs.begin(), avoidPointIDs.end());
  return avoidPointIDs;
}
