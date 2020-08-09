#include "netlist_paths/NetlistPaths.hpp"

using namespace netlist_paths;

/// Determine the max length of a name.
int NetlistPaths::maxNameLength(const std::vector<VertexDesc> &names) const {
  size_t maxLength = 0;
  for (auto v : names) {
    if (netlist.getVertex(v).canIgnore()) {
      continue;
    }
    maxLength = std::max(maxLength, netlist.getVertex(v).name.size());
  }
  return static_cast<int>(maxLength);
}

/// Dump unique names of vars/regs/wires in the netlist for searching.
std::vector<std::reference_wrapper<const Vertex>> NetlistPaths::getNamedVertices() const {
  std::vector<std::reference_wrapper<const Vertex>> vertices;
  // Collect vertices.
  for (auto vertexId : netlist.getAllVertices()) {
    if (netlist.getVertex(vertexId).isVisible()) {
      vertices.push_back(std::ref(netlist.getVertex(vertexId)));
    }
  }
  // Sort them.
  auto compare = [](const Vertex& a, const Vertex& b) {
                   return a.compareLessThan(b); };
  std::sort(vertices.begin(), vertices.end(), compare);
  return vertices;
}

void NetlistPaths::printNames() const {
  //auto names = netlist->getNames();
  //// Print the output.
  //int maxWidth = netlist->maxNameLength(names) + 1;
  //std::cout << std::left << std::setw(maxWidth) << "Name"
  //          << std::left << std::setw(10)       << "Type"
  //          << std::left << std::setw(10)       << "Direction"
  //          << std::left << std::setw(10)       << "Width"
  //                                              << "Location\n";
  //for (auto v : names) {
  //  auto type = getVertexAstTypeStr(graph[v].astType);
  //  auto srcPath = netlist_paths::options.fullFileNames ? fs::path(graph[v].location.getFilename())
  //                                                      : fs::path(graph[v].location.getFilename()).filename();
  //  std::cout << std::left << std::setw(maxWidth) << graph[v].name
  //            << std::left << std::setw(10)       << (std::string(type) == "REG_DST" ? "REG" : type)
  //            << std::left << std::setw(10)       << getVertexDirectionStr(graph[v].direction)
  //            //<< std::left << std::setw(10)       << graph[v].width
  //                                                << srcPath.string()
  //            << "\n";
  //}
}

///// Pretty print a path (some sequence of vertices).
//void Netlist::printPathReport(const Path &path) const {
//  int maxWidth = maxNameLength(path) + 1;
//  // Print each vertex on the path.
//  for (auto v : path) {
//    if (canIgnore(graph[v]))
//      continue;
//    auto srcPath = netlist_paths::options.fullFileNames ? fs::path(graph[v].location.getFilename())
//                                                        : fs::path(graph[v].location.getFilename()).filename();
//    if (!netlist_paths::options.reportLogic) {
//      if (!isLogic(graph[v])) {
//        std::cout << "  " << std::left
//                  << std::setw(maxWidth)
//                  << graph[v].name
//                  << srcPath.string() << "\n";
//      }
//    } else {
//      if (isLogic(graph[v])) {
//        std::cout << "  " << std::left
//                  << std::setw(maxWidth)
//                  << getVertexAstTypeStr(graph[v].type)
//                  << std::setw(VERTEX_TYPE_STR_MAX_LEN)
//                  << "LOGIC"
//                  << srcPath.string() << "\n";
//      } else {
//        std::cout << "  " << std::left
//                  << std::setw(maxWidth)
//                  << graph[v].name
//                  << std::setw(VERTEX_TYPE_STR_MAX_LEN)
//                  << getVertexAstTypeStr(graph[v].type)
//                  << srcPath.string() << "\n";
//      }
//    }
//  }
//}
//
///// Print a collection of paths.
//void Netlist::
//printPathReport(const std::vector<Path> &paths) const {
//  int pathCount = 0;
//  for (auto &path : paths) {
//    if (!path.empty()) {
//      std::cout << "Path " << ++pathCount << "\n";
//      printPathReport(path);
//      std::cout << "\n";
//    }
//  }
//  std::cout << "Found " << pathCount << " path(s)\n";
//}
//
