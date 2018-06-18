#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/program_options.hpp>

#ifdef NDEBUG
#define DEBUG(x) do {} while (false)
#else
#define DEBUG(x) do { if (debugMode) { x; } } while (false)
#endif

namespace po = boost::program_options;

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
using ParentMap = std::map<int, std::vector<int>>;

bool debugMode;

struct Exception : public std::exception {
   std::string msg;
   Exception(std::string msg) : msg(msg) {}
   ~Exception() throw () {}
   const char* what() const throw() { return msg.c_str(); }
};

typedef enum {
  NONE,
  INPUTS,
  REG,
  VAR,
  VAR_STD,
  VAR_PRE,
  VAR_POST,
  VAR_PORD,
  LOGIC,
  LOGICACTIVE,
  LOGICASSIGNW,
  LOGICASSIGNPRE,
  LOGICASSIGNPOST,
  LOGICALWAYS
} VertexType;

VertexType getVertexType(const std::string &type) {
  if (type == "*INPUTS*")             return INPUTS;
  else if (type == "REG")             return REG;
  else if (type == "VAR")             return VAR;
  else if (type == "VAR_STD")         return VAR_STD;
  else if (type == "VAR_PRE")         return VAR_PRE;
  else if (type == "VAR_POST")        return VAR_POST;
  else if (type == "VAR_PORD")        return VAR_PORD;
  else if (type == "LOGIC")           return LOGIC;
  else if (type == "LOGICACTIVE")     return LOGICACTIVE;
  else if (type == "LOGICASSIGNW")    return LOGICASSIGNW;
  else if (type == "LOGICASSIGNPRE")  return LOGICASSIGNPRE;
  else if (type == "LOGICASSIGNPOST") return LOGICASSIGNPOST;
  else if (type == "LOGICALWAYS")     return LOGICALWAYS;
  else {
    throw Exception(std::string("unexpected vertex type: ")+type);
  }
}

const char *getVertexTypeStr(VertexType type) {
  switch (type) {
    case INPUTS:         return "*INPUTS*";
    case REG:            return "REG";
    case VAR:            return "VAR";
    case VAR_STD:        return "VAR_STD";
    case VAR_PRE:        return "VAR_PRE";
    case VAR_POST:       return "VAR_POST";
    case VAR_PORD:       return "VAR_PORD";
    case LOGIC:          return "LOGIC";
    case LOGICACTIVE:    return "LOGICACTIVE";
    case LOGICASSIGNW:   return "LOGICASSIGNW";
    case LOGICASSIGNPRE: return "LOGICASSIGNPRE";
    case LOGICASSIGNPOST:return "LOGICASSIGNPOST";
    case LOGICALWAYS:    return "LOGICALWAYS";
    default:             return "UNKNOWN";
  }
}

struct Vertex {
  int id;
  VertexType type;
  std::string name;
  std::string loc;
  Vertex(int id, VertexType type) :
    id(id), type(type) {}
  Vertex(int id, VertexType type, const std::string &loc) :
    id(id), type(type), loc(loc) {}
  Vertex(int id, VertexType type,
         const std::string &name,
         const std::string &loc) :
    id(id), type(type), name(name), loc(loc) {}
};

struct Edge {
  int src;
  int dst;
  Edge(int src, int dst) : src(src), dst(dst) {}
};

class DfsVisitor : public boost::default_dfs_visitor {
private:
  ParentMap &parentMap;
public:
  DfsVisitor(ParentMap &parentMap) : parentMap(parentMap) {}
  template<typename Edge, typename Graph>
  // Visit only the edges of the DFS graph.
  void tree_edge(Edge edge, const Graph &graph) const {
    typename boost::graph_traits<Graph>::vertex_descriptor src, dst;
    src = boost::source(edge, graph);
    dst = boost::target(edge, graph);
    parentMap[dst].push_back(src);
    return;
  }
};

class DfsVisitorAllPaths : public boost::default_dfs_visitor {
private:
  ParentMap &parentMap;
public:
  DfsVisitorAllPaths(ParentMap &parentMap) : parentMap(parentMap) {}
  template<typename Edge, typename Graph>
  // Visit all edges of a all vertices.
  void examine_edge(Edge edge, const Graph &graph) const {
    typename boost::graph_traits<Graph>::vertex_descriptor src, dst;
    src = boost::source(edge, graph);
    dst = boost::target(edge, graph);
    parentMap[dst].push_back(src);
    return;
  }
};

int getVertexId(const std::vector<Vertex> vertices,
                const std::string &name,
                VertexType type) {
  auto pred = [&] (const Vertex &v) { return v.type == type &&
                                             v.name == name; };
  auto it = std::find_if(std::begin(vertices), std::end(vertices), pred);
  if (it == vertices.end()) {
    throw Exception(std::string("could not find vertex ")
                        +name+" of type "+getVertexTypeStr(type));
  }
  DEBUG(std::cout<<"Vertex "<<it->id<<" matches "<<name
                 <<" of type "<<getVertexTypeStr(type)<<"\n");
  return it->id;
}

void dumpPath(const std::vector<Vertex> vertices,
              const std::vector<int> path) {
  for (int vertexId : path) {
    if (vertices[vertexId].type != LOGIC) {
      std::cout << "  " << vertices[vertexId].name << "\n";
    }
  }
}

void determinePaths(std::vector<Vertex> &vertices,
                    ParentMap &parentMap,
                    std::vector<std::vector<int>> &result,
                    std::vector<int> path,
                    int startVertexId,
                    int endVertexId) {
  path.push_back(endVertexId);
  if (endVertexId == startVertexId) {
    DEBUG(std::cout << "FINISH\n");
    result.push_back(path);
    return;
  }
  DEBUG(std::cout<<"length "<<path.size()<<" vertex "<<endVertexId<<"\n");
  DEBUG(dumpPath(vertices, path));
  DEBUG(std::cout<<(parentMap[endVertexId].empty()?"DEAD END\n":""));
  for (auto &vertex : parentMap[endVertexId]) {
    if (std::find(path.begin(), path.end(), vertex) == path.end()) {
      determinePaths(vertices, parentMap, result, path, startVertexId, vertex);
    } else {
      DEBUG(std::cout << "CYCLE\n");
    }
  }
}

void parseFile(std::string &filename,
               std::vector<Vertex> &vertices,
               std::vector<Edge> &edges) {
  std::fstream infile(filename);
  std::string line;
  int vertexCount = 0;
  if (!infile.is_open()) {
    throw Exception("could not open file");
  }
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
    if (tokens[0] == "VERTEX") {
      auto type = getVertexType(tokens[2]);
      if (type == INPUTS) {
        // No name or fileline.
        vertices.push_back(Vertex(vertexCount, type));
      } else if (type == LOGICACTIVE ||
                 type == LOGICASSIGNW ||
                 type == LOGICASSIGNPRE ||
                 type == LOGICASSIGNPOST ||
                 type == LOGICALWAYS) {
        // No name.
        vertices.push_back(Vertex(vertexCount, type, tokens[4]));
      } else {
        vertices.push_back(Vertex(vertexCount, type, tokens[3], tokens[5]));
      }
      ++vertexCount;
    } else if (tokens[0] == "EDGE") {
      auto edge = Edge(std::stoi(tokens[1]),
                       std::stoi(tokens[3]));
      edges.push_back(edge);
    } else {
      throw Exception(std::string("unexpected line: ")+line);
    }
  }
}

void dumpDotFile(std::vector<Vertex> vertices, std::vector<Edge> edges) {
  std::cout << "digraph netlist_graph {\n";
  for (auto &vertex : vertices)
    std::cout << "  " << vertex.id << " [label=\""
              << getVertexTypeStr(vertex.type) << "\\n"
              << vertex.name << "\"];\n";
  for (auto &edge : edges)
    std::cout << "  " << edge.src << " -> " << edge.dst << ";\n";
  std::cout << "}\n";
}

void reportPath(std::vector<Vertex> &vertices, std::vector<int> path) {
  for (auto &vertexId : path) {
    auto &vertex = vertices[vertexId];
    std::cout << "  " << std::left
              << std::setw(64) << vertex.name
              << std::setw(8) << getVertexTypeStr(vertex.type) << " "
              << vertex.loc << "\n";
  }
}

int main(int argc, char **argv) {
  try {
    // Command line options.
    std::string inputFile;
    std::string startName;
    std::string endName;
    po::options_description hiddenOptions("Positional options");
    po::options_description genericOptions("General options");
    po::options_description allOptions("All options");
    po::positional_options_description p;
    po::variables_map vm;
    // Specify options.
    hiddenOptions.add_options()
      ("input-file", po::value<std::string>(&inputFile)->required());
    p.add("input-file", 1);
    genericOptions.add_options()
      ("help,h",   "display help")
      ("debug",    "print debugging information")
      ("allpaths", "find all paths (exponential time)")
      ("dotfile",  "dump dotfile")
      ("start,s",  po::value<std::string>(&startName), "Start point")
      ("end,e",    po::value<std::string>(&endName),   "End point");
    allOptions.add(genericOptions).add(hiddenOptions);
    // Parse command line.
    po::store(po::command_line_parser(argc, argv).
                  options(allOptions).positional(p).run(), vm);
    debugMode = vm.count("debug") > 0;
    bool displayHelp = vm.count("help");
    bool dumpDotfile = vm.count("dotfile");
    bool allPaths    = vm.count("allpaths");
    if (displayHelp) {
      std::cout << "OVERVIEW: Find paths in a Verilog netlist\n\n";
      std::cout << "USAGE: " << argv[0] << " [options] infile\n\n";
      std::cout << genericOptions << "\n";
      return 1;
    }
    notify(vm);
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    ParentMap parentMap;
    // Parse the input file.
    DEBUG(std::cout << "Parsing input file\n");
    parseFile(inputFile, vertices, edges);
    // Dump dot file.
    if (dumpDotfile) {
      dumpDotFile(vertices, edges);
      return 0;
    }
    // Construct graph.
    DEBUG(std::cout << "Constructing graph\n");
    Graph graph(vertices.size());
    for (auto &edge : edges) {
      if (!boost::edge(edge.src, edge.dst, graph).second) {
        boost::add_edge(edge.src, edge.dst, graph);
      }
    }
    // Find and report paths.
    if (startName.empty())
      throw Exception("no start point specified");
    if (endName.empty())
      throw Exception("no end point specified");
    int startVertexId = getVertexId(vertices, startName, VAR);
    int endVertexId = getVertexId(vertices, endName, REG);
    DEBUG(std::cout << "Performing DFS\n");
    if (allPaths) {
      boost::depth_first_search(graph,
        boost::visitor(DfsVisitorAllPaths(parentMap)).root_vertex(startVertexId));
    } else {
      boost::depth_first_search(graph,
        boost::visitor(DfsVisitor(parentMap)).root_vertex(startVertexId));
    }
    DEBUG(std::cout << "Determining paths\n");
    std::vector<std::vector<int>> paths;
    determinePaths(vertices, parentMap, paths, std::vector<int>(),
                   startVertexId, endVertexId);
    int count = 0;
    for (auto &path : paths) {
      std::reverse(path.begin(), path.end());
      std::cout << "PATH " << ++count << ":\n";
      reportPath(vertices, path);
    }
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
