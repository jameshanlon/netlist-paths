#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

typedef enum {
  NONE,
  INPUTS,
  VAR_STD,
  VAR_PRE,
  VAR_POST,
  VAR_PORD,
  LOGICACTIVE,
  LOGICASSIGNW,
  LOGICASSIGNPRE,
  LOGICASSIGNPOST,
  LOGICALWAYS
} VertexType;

struct Vertex {
  int id;
  VertexType type;
  std::string name;
  std::string loc;
  Vertex(int id, VertexType type, const std::string &name, const std::string &loc) :
    id(id), type(type), name(name), loc(loc) {}
};

struct Edge {
  int src;
  int dst;
  Edge(int src, int dst) : src(src), dst(dst) {}
};

using Graph = boost::adjacency_list<boost::listS, boost::vecS>;

class DfsVisitor : public boost::default_dfs_visitor {
public:
  template<typename Vertex, typename Graph>
  void discover_vertex(Vertex vertex, const Graph &graph) const {
    std::cout << vertex << "\n";
    return;
  }
};

VertexType getVertexType(const std::string &type) {
  if (type == "*INPUTS*") return INPUTS;
  else if (type == "VAR_STD")         return VAR_STD;
  else if (type == "VAR_PRE")         return VAR_PRE;
  else if (type == "VAR_POST")        return VAR_POST;
  else if (type == "VAR_PORD")        return VAR_PORD;
  else if (type == "LOGICACTIVE")     return LOGICACTIVE;
  else if (type == "LOGICASSIGNW")    return LOGICASSIGNW;
  else if (type == "LOGICASSIGNPRE")  return LOGICASSIGNPRE;
  else if (type == "LOGICASSIGNPOST") return LOGICASSIGNPOST;
  else if (type == "LOGICALWAYS")     return LOGICALWAYS;
  else {
    std::cout << "Unexpected vertex type: " << type << "\n";
    return NONE;
  }
}

int main(int argc, char **argv) {
  std::fstream infile(argv[1]);
  if (!infile.is_open()) {
    std::cout << "Could not open file\n";
    std::exit(1);
  }
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
  std::string line;
  int vertexCount = 0;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
    if (tokens[0] == "VERTEX") {
      auto type = getVertexType(tokens[2]);
      auto vertex = Vertex(vertexCount++, type, tokens[3], tokens[5]);
      vertices.push_back(vertex);
    }
    else if (tokens[0] == "EDGE") {
      auto edge = Edge(std::stoi(tokens[1]),
                       std::stoi(tokens[3]));
      edges.push_back(edge);
    } else {
      std::cout << "Unexpected line: " << line;
      std::exit(1);
    }
  }
  Graph graph(vertices.size());
  for (auto &edge : edges) {
    boost::add_edge(edge.src, edge.dst, graph);
  }
  DfsVisitor visitor;
  boost::depth_first_search(graph, boost::visitor(visitor));
  return 0;
}
