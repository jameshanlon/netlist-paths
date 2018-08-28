#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/program_options.hpp>

#ifdef NDEBUG
#define DEBUG(x) do {} while (false)
#else
#define DEBUG(x) do { if (debugMode) { x; } } while (false)
#endif

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace bp = boost::process;

namespace {

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
  bool allPaths;
public:
  DfsVisitor(ParentMap &parentMap, bool allPaths) :
      parentMap(parentMap), allPaths(allPaths) {}
  // Visit only the edges of the DFS graph.
  template<typename Edge, typename Graph>
  void tree_edge(Edge edge, const Graph &graph) const {
    if (!allPaths) {
      typename boost::graph_traits<Graph>::vertex_descriptor src, dst;
      src = boost::source(edge, graph);
      dst = boost::target(edge, graph);
      parentMap[dst].push_back(src);
    }
    return;
  }
  // Visit all edges of a all vertices.
  template<typename Edge, typename Graph>
  void examine_edge(Edge edge, const Graph &graph) const {
    if (allPaths) {
      typename boost::graph_traits<Graph>::vertex_descriptor src, dst;
      src = boost::source(edge, graph);
      dst = boost::target(edge, graph);
      parentMap[dst].push_back(src);
    }
    return;
  }
};

int getVertexId(const std::vector<Vertex> vertices,
                const std::string &name,
                VertexType type) {
  auto pred = [&] (const Vertex &v) { return v.type == type &&
                                             v.name == name; };
  auto it = std::find_if(std::begin(vertices), std::end(vertices), pred);
  if (it == std::end(vertices)) {
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

/// Given the tree structure from a DFS, traverse the tree from leaf to root to
/// return a path.
std::vector<int> determinePath(ParentMap &parentMap,
                               std::vector<int> path,
                               int startVertexId,
                               int endVertexId) {
  path.push_back(endVertexId);
  if (endVertexId == startVertexId) {
    return path;
  }
  if (parentMap[endVertexId].size() == 0)
    return std::vector<int>();
  assert(parentMap[endVertexId].size() == 1);
  int nextVertexId = parentMap[endVertexId].front();
  assert(std::find(std::begin(path), std::end(path), nextVertexId) == std::end(path));
  return determinePath(parentMap, path, startVertexId, nextVertexId);
}

/// Determine all paths between a start and an end point.
/// This performs a DFS starting at the end point. It is not feasible for large
/// graphs since the number of simple paths grows exponentially.
void determineAllPaths(const std::vector<Vertex> &vertices,
                       ParentMap &parentMap,
                       std::vector<std::vector<int>> &result,
                       std::vector<int> path,
                       int startVertexId,
                       int endVertexId) {
  path.push_back(endVertexId);
  if (endVertexId == startVertexId) {
    DEBUG(std::cout << "FOUND PATH\n");
    result.push_back(path);
    return;
  }
  DEBUG(std::cout<<"length "<<path.size()<<" vertex "<<endVertexId<<"\n");
  DEBUG(dumpPath(vertices, path));
  DEBUG(std::cout<<(parentMap[endVertexId].empty()?"DEAD END\n":""));
  for (auto &vertex : parentMap[endVertexId]) {
    if (std::find(std::begin(path), std::end(path), vertex) == std::end(path)) {
      determineAllPaths(vertices, parentMap, result, path,
                        startVertexId, vertex);
    } else {
      DEBUG(std::cout << "CYCLE DETECTED\n");
    }
  }
}

void parseFile(const std::string &filename,
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

Graph buildGraph(std::vector<Vertex> &vertices,
                 std::vector<Edge> &edges) {
  // Construct graph.
  DEBUG(std::cout << "Constructing graph\n");
  Graph graph(vertices.size());
  for (auto &edge : edges) {
    if (!boost::edge(edge.src, edge.dst, graph).second) {
      boost::add_edge(edge.src, edge.dst, graph);
    }
  }
  return graph;
}

void dumpDotFile(const std::vector<Vertex> vertices,
                 const std::vector<Edge> edges) {
  std::cout << "digraph netlist_graph {\n";
  for (auto &vertex : vertices)
    std::cout << "  " << vertex.id << " [label=\""
              << getVertexTypeStr(vertex.type) << "\\n"
              << vertex.name << "\"];\n";
  for (auto &edge : edges)
    std::cout << "  " << edge.src << " -> " << edge.dst << ";\n";
  std::cout << "}\n";
}

void printPathReport(const std::vector<Vertex> &vertices,
                     const std::vector<int> path,
                     bool netsOnly=false) {
  // Determine the max length of a name.
  size_t maxNameLength = 0;
  for (auto &vertexId : path) {
    maxNameLength = std::max(maxNameLength, vertices[vertexId].name.size());
  }
  // Print each vertex on the path.
  for (auto &vertexId : path) {
    auto &vertex = vertices[vertexId];
    if (netsOnly) {
      if (vertex.type == LOGIC)
        continue;
      std::cout << "  " << std::left
                << std::setw(maxNameLength+1) << vertex.name
                << vertex.loc << "\n";
    } else {
      std::cout << "  " << std::left
                << std::setw(maxNameLength+1) << vertex.name
                << std::setw(8) << getVertexTypeStr(vertex.type) << " "
                << vertex.loc << "\n";
    }
  }
}

int compileGraph(const std::vector<std::string> includes,
                 const std::vector<std::string> defines,
                 const std::vector<std::string> inputFiles) {
  fs::path exe = fs::system_complete("verilator/bin/verilator_bin");
  std::vector<std::string> args{"+1800-2012ext+.sv",
                                "--lint-only",
                                "--dump-netlist-graph",
                                "--error-limit", "10000"};
  for (auto &path : includes)
    args.push_back(std::string("+incdir+")+path);
  for (auto &define : defines)
    args.push_back(std::string("-D")+define);
  for (auto &path : inputFiles)
    args.push_back(path);
  std::stringstream ss;
  for (auto &arg : args)
    ss << arg << " ";
  DEBUG(std::cout << "Running: " << exe << " " << ss.str() << "\n");
  return bp::system(exe, bp::args(args));
}

void reportAllFanout(const std::string &startName,
                     const std::vector<Vertex> &vertices,
                     const std::vector<Edge> &edges,
                     Graph &graph,
                     bool netsOnly=false) {
  int startVertexId = getVertexId(vertices, startName, VAR);
  DEBUG(std::cout << "Performing DFS from " << startVertexId << "\n");
  ParentMap parentMap;
  boost::depth_first_search(graph,
      boost::visitor(DfsVisitor(parentMap, false))
        .root_vertex(startVertexId));
  // Check for a path between startPoint and each register.
  for (auto &vertex : vertices) {
    if (vertex.type == REG) {
      auto path = determinePath(parentMap, std::vector<int>(),
                                startVertexId, vertex.id);
      if (!path.empty())
        printPathReport(vertices, path, netsOnly);
    }
  }
}

void reportAnyPointToPoint(Graph &graph,
                           const std::vector<int> waypoints,
                           const std::vector<Vertex> vertices,
                           bool netsOnly=false) {
  std::vector<int> path;
  // Construct the path between each adjacent waypoints.
  for (size_t i = 0; i < waypoints.size()-1; ++i) {
    int startVertexId = waypoints[i];
    int endVertexId = waypoints[i+1];
    DEBUG(std::cout << "Performing DFS from " << startVertexId << "\n");
    ParentMap parentMap;
    boost::depth_first_search(graph,
        boost::visitor(DfsVisitor(parentMap, false))
          .root_vertex(startVertexId));
    DEBUG(std::cout << "Determining a path to " << endVertexId << "\n");
    auto subPath = determinePath(parentMap, std::vector<int>(),
                                 startVertexId, endVertexId);
    if (subPath.empty())
        throw Exception(std::string("no path from ")
                            +std::to_string(startVertexId)+" to "
                            +std::to_string(endVertexId));
    std::reverse(std::begin(subPath), std::end(subPath));
    path.insert(std::end(path), std::begin(subPath), std::end(subPath)-1);
  }
  path.push_back(waypoints.back());
  printPathReport(vertices, path, netsOnly);
}

void reportAllPointToPoint(Graph &graph,
                           const std::vector<int> waypoints,
                           const std::vector<Vertex> vertices,
                           bool netsOnly=false) {
  if (waypoints.size() > 2)
    throw Exception("through points not supported for all paths");
  DEBUG(std::cout << "Performing DFS\n");
  ParentMap parentMap;
  boost::depth_first_search(graph,
      boost::visitor(DfsVisitor(parentMap, true))
        .root_vertex(waypoints[0]));
  DEBUG(std::cout << "Determining all paths\n");
  std::vector<std::vector<int>> paths;
  determineAllPaths(vertices, parentMap, paths, std::vector<int>(),
                    waypoints[0], waypoints[1]);
  int count = 0;
  for (auto &path : paths) {
    std::reverse(std::begin(path), std::end(path));
    std::cout << "PATH " << ++count << ":\n";
    printPathReport(vertices, path, netsOnly);
  }
}

} // End anonymous namespace.

int main(int argc, char **argv) {
  try {
    // Command line options.
    std::vector<std::string> inputFiles;
    std::string startName;
    std::string endName;
    std::vector<std::string> throughNames;
    po::options_description hiddenOptions("Positional options");
    po::options_description genericOptions("General options");
    po::options_description allOptions("All options");
    po::positional_options_description p;
    po::variables_map vm;
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::vector<int> waypoints;
    // Specify command line options.
    hiddenOptions.add_options()
      ("input-file",
       po::value<std::vector<std::string>>(&inputFiles)->required());
    p.add("input-file", -1);
    genericOptions.add_options()
      ("help,h",    "Display help")
      ("start,s",   po::value<std::string>(&startName), "Start point")
      ("end,e",     po::value<std::string>(&endName),   "End point")
      ("through,t", po::value<std::vector<std::string>>(&throughNames),
       "Through point")
      ("allpaths",  "Find all paths between two points (exponential time)")
      ("netsonly",  "Only display nets in path report")
      ("compile",   "Compile a netlist graph from Verilog source")
      ("include,I", po::value<std::vector<std::string>>()->composing(),
                    "include path (only with --compile)")
      ("define,D",  po::value<std::vector<std::string>>()->composing(),
                    "define a preprocessor macro (only with --compile)")
      ("dotfile",   "Dump dotfile of netlist graph")
      ("debug",     "Print debugging information");
    allOptions.add(genericOptions).add(hiddenOptions);
    // Parse command line arguments.
    po::store(po::command_line_parser(argc, argv).
                  options(allOptions).positional(p).run(), vm);
    debugMode = vm.count("debug") > 0;
    bool displayHelp = vm.count("help");
    bool dumpDotfile = vm.count("dotfile");
    bool allPaths    = vm.count("allpaths");
    bool netsOnly    = vm.count("netsonly");
    bool compile     = vm.count("compile");
    if (displayHelp) {
      std::cout << "OVERVIEW: Query paths in a Verilog netlist\n\n";
      std::cout << "USAGE: " << argv[0] << " [options] infile\n\n";
      std::cout << genericOptions << "\n";
      return 1;
    }
    notify(vm);
    // Call Verilator to produce graph file. ================================ //
    if (compile) {
      auto includes = vm.count("include")
                        ? vm["include"].as<std::vector<std::string>>()
                        : std::vector<std::string>{};
      auto defines = vm.count("define")
                        ? vm["define"].as<std::vector<std::string>>()
                        : std::vector<std::string>{};
      return compileGraph(includes, defines, inputFiles);
    }
    // Parse the input file. =================================================//
    if (inputFiles.size() != 1)
      throw Exception("multiple graph files specified");
    DEBUG(std::cout << "Parsing input file\n");
    parseFile(inputFiles.front(), vertices, edges);
    // Dump dot file. ========================================================//
    if (dumpDotfile) {
      dumpDotFile(vertices, edges);
      return 0;
    }
    // Build the graph. ======================================================//
    auto graph = buildGraph(vertices, edges);
    // Any query must have a start point.
    if (startName.empty())
      throw Exception("no start point specified");
    // Report paths fanning out from startName.===============================//
    if (endName.empty()) {
      if (!throughNames.empty())
        throw Exception("through points not supported for start only");
      reportAllFanout(startName, vertices, edges, graph, netsOnly);
      return 0;
    }
    // Find vertices in graph and compile path waypoints. ====================//
    waypoints.push_back(getVertexId(vertices, startName, VAR));
    for (auto &throughName : throughNames) {
      int vertexId = getVertexId(vertices, throughName, VAR);
      waypoints.push_back(vertexId);
    }
    waypoints.push_back(getVertexId(vertices, endName, REG));
    // Report all paths between two points. ==================================//
    if (allPaths) {
      reportAllPointToPoint(graph, waypoints, vertices, netsOnly);
      return 0;
    }
    // Report a paths between two points. ====================================//
    reportAnyPointToPoint(graph, waypoints, vertices, netsOnly);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
