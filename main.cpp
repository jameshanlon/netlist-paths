#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

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

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
using ParentMap = std::map<int, std::vector<int>>;

struct Exception : public std::exception {
   std::string msg;
   Exception(std::string msg) : msg(msg) {}
   ~Exception() throw () {}
   const char* what() const throw() { return msg.c_str(); }
};

class DfsVisitor : public boost::default_dfs_visitor {
private:
  ParentMap &parentMap;
public:
  DfsVisitor(ParentMap &parentMap) : parentMap(parentMap) {}
  template<typename Edge, typename Graph>
  void tree_edge(Edge edge, const Graph &graph) const {
    //std::cout << edge << "\n";
    typename boost::graph_traits<Graph>::vertex_descriptor src, dst;
    src = boost::source(edge, graph);
    dst = boost::target(edge, graph);
    parentMap[dst].push_back(src);
    return;
  }
};

VertexType getVertexType(const std::string &type) {
  if (type == "*INPUTS*")             return INPUTS;
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
    throw Exception(std::string("Unexpected vertex type: ")+type);
  }
}

int getVertexId(const std::vector<Vertex> vertices, const std::string &name) {
  auto pred = [&] (const Vertex &v) { return v.type == VAR_STD &&
                                             v.name == name; };
  auto it = std::find_if(std::begin(vertices), std::end(vertices), pred);
  return it->id;
}

void reportPaths(ParentMap &parentMap, std::vector<int> path, int startVertexId, int endVertexId) {
  if (endVertexId == startVertexId) {
    std::cout << "PATH: ";
    for (auto vertexId : path)
      std::cout << vertexId << " ";
  }
  path.push_back(endVertexId);
  for (auto &vertex : parentMap[endVertexId]) {
    reportPaths(parentMap, path, startVertexId, vertex);
  }
}

int main(int argc, char **argv) {
  try {
    // Command line options.
    std::string inputFile;
    std::string startName;
    std::string endName;
    po::options_description positionalDesc("Positional");
    po::options_description generalDesc("General options");
    po::options_description all("All");
    po::positional_options_description positionalOpts;
    po::variables_map vm;
    // Specify options.
    positionalDesc.add_options()
      ("infile.graph", po::value<std::string>(&inputFile)->required());
    positionalOpts.add("infile.graph", 1);
    generalDesc.add_options()
      ("help,h", "display help")
      ("start,s", po::value<std::string>(&startName), "Start point")
      ("end,e", po::value<std::string>(&endName), "End point");
    all.add(positionalDesc).add(generalDesc);
    // Parse command line.
    po::store(po::parse_command_line(argc, argv, all), vm);
    if (vm.count("help")) {
      std::cout << "OVERVIEW: Find paths in a Verilog netlist\n\n";
      std::cout << "USAGE: " << argv[0] << " infile [options]\n\n";
      std::cout << generalDesc << "\n";
      return 1;
    }
    std::fstream infile(inputFile);
    if (!infile.is_open()) {
      throw Exception("Could not open file");
    }
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    ParentMap parentMap;
    // Parse file.
    std::string line;
    int vertexCount = 0;
    while (std::getline(infile, line)) {
      std::istringstream iss(line);
      std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>{}};
      if (tokens[0] == "VERTEX") {
        auto type = getVertexType(tokens[2]);
        auto vertex = Vertex(vertexCount, type, tokens[3], tokens[5]);
        vertices.push_back(vertex);
        ++vertexCount;
      } else if (tokens[0] == "EDGE") {
        auto edge = Edge(std::stoi(tokens[1]),
                         std::stoi(tokens[3]));
        edges.push_back(edge);
      } else {
        throw Exception(std::string("Unexpected line: ")+line);
      }
    }
    // Construct graph.
    Graph graph(vertices.size());
    for (auto &edge : edges) {
      boost::add_edge(edge.src, edge.dst, graph);
    }
    // Find and report paths.
    int startVertexId = getVertexId(vertices, startName);
    int endVertexId = getVertexId(vertices, endName);
    DfsVisitor visitor(parentMap);
    boost::depth_first_search(graph, boost::visitor(visitor).root_vertex(startVertexId));
    reportPaths(parentMap, std::vector<int>(), startVertexId, endVertexId);
    return 0;
  } catch (po::required_option&) {
    std::cerr << "Error: no input file.\n";
    return 1;
  } catch (Exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  } catch (std::exception& e) {
    std::cerr << "Unexpected error: " << e.what() << "\n";
    return 1;
  }
}
