#include <regex>
#include <boost/format.hpp>
#include "netlist_paths/Netlist.hpp"

using namespace netlist_paths;

/// Return a list of IDs of named vertices, optionally filter by regex.
std::vector<VertexID>
Netlist::getNamedVertexIds(const std::string &regex) const {
  std::vector<VertexID> vertexIds;
  std::regex nameRegex(regex);
  // Collect vertices.
  for (auto vertexId : netlist.getAllVertices()) {
    if (netlist.getVertex(vertexId).isNamed() &&
        (regex.empty() ||
         std::regex_search(netlist.getVertex(vertexId).getName(), nameRegex))) {
      vertexIds.push_back(vertexId);
    }
  }
  return vertexIds;
}

/// Return a sorted list of unique named entities in the netlist for searching.
std::vector<std::reference_wrapper<const Vertex>>
Netlist::getNamedVertices(const std::string &regex) const {
  // Collect vertices.
  std::vector<std::reference_wrapper<const Vertex>> vertices;
  for (auto vertexId : getNamedVertexIds(regex)) {
    vertices.push_back(std::ref(netlist.getVertex(vertexId)));
  }
  // Sort them.
  auto compare = [](const Vertex& a, const Vertex& b) {
                   return a.compareLessThan(b); };
  std::sort(vertices.begin(), vertices.end(), compare);
  return vertices;
}

/// Dump details of named entities in the design.
void Netlist::dumpNames(std::ostream &os, const std::string &regex) const {
  // Populate the rows of the table.
  std::vector<const std::string> hdr({"Name", "Type", "DType", "Direction", "Location"});
  std::vector<std::vector<const std::string>> rows;
  for (auto vertexId : getNamedVertexIds(regex)) {
    rows.push_back({netlist.getVertex(vertexId).getName(),
                    netlist.getVertex(vertexId).getAstTypeStr(),
                    netlist.getVertex(vertexId).getDTypeStr(),
                    netlist.getVertex(vertexId).getDirStr(),
                    netlist.getVertex(vertexId).getLocStr()});
  }
  // Create the row format string.
  std::string fmt;
  for (size_t i=0; i<hdr.size(); i++) {
    size_t maxWidth = hdr[i].size();
    for (size_t j=0; j<rows.size(); j++) {
      maxWidth = std::max(maxWidth, rows[j][i].size());
    }
    fmt += "%-"+std::to_string(maxWidth+1)+"s ";
  }
  fmt += '\n';
  os << boost::format(fmt) % hdr[0] % hdr[1] % hdr[2] % hdr[3] % hdr[4];
  for (auto row : rows) {
    os << boost::format(fmt) % row[0] % row[1] % row[2] % row[3] % row[4];
  }
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
