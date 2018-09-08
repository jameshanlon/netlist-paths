#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
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

using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
                                    boost::bidirectionalS>;
using ParentMap = std::map<int, std::vector<int>>;

const int NULL_VERTEX_ID = 0;
const char *DEFAULT_OUTPUT_FILENAME = "netlist";

bool debugMode;

struct Exception : public std::exception {
   std::string msg;
   Exception(std::string msg) : msg(msg) {}
   ~Exception() throw () {}
   const char* what() const throw() { return msg.c_str(); }
};

typedef enum {
  NONE,
  LOGIC,
  REG_SRC,
  REG_DST,
  REG_DST_OUTPUT,
  VAR,
  VAR_WIRE,
  VAR_INPUT,
  VAR_OUTPUT,
  VAR_INOUT,
} VertexType;

VertexType getVertexType(const std::string &type) {
       if (type == "LOGIC")          return LOGIC;
  else if (type == "REG_SRC")        return REG_SRC;
  else if (type == "REG_DST")        return REG_DST;
  else if (type == "REG_DST_OUTPUT") return REG_DST_OUTPUT;
  else if (type == "VAR")            return VAR;
  else if (type == "VAR_WIRE")       return VAR_WIRE;
  else if (type == "VAR_INPUT")      return VAR_INPUT;
  else if (type == "VAR_OUTPUT")     return VAR_OUTPUT;
  else if (type == "VAR_INOUT")      return VAR_INOUT;
  else {
    throw Exception(std::string("unexpected vertex type: ")+type);
  }
}

const char *getVertexTypeStr(VertexType type) {
  switch (type) {
    case LOGIC:          return "LOGIC";
    case REG_SRC:        return "REG_SRC";
    case REG_DST:        return "REG_DST";
    case REG_DST_OUTPUT: return "REG_DST_OUTPUT";
    case VAR:            return "VAR";
    case VAR_WIRE:       return "VAR_WIRE";
    case VAR_INPUT:      return "VAR_INPUT";
    case VAR_OUTPUT:     return "VAR_OUTPUT";
    case VAR_INOUT:      return "VAR_INOUT";
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

/// Parse a graph input file and return a list of Vertices and a list of Edges.
void parseFile(const std::string &filename,
               std::vector<Vertex> &vertices,
               std::vector<Edge> &edges) {
  DEBUG(std::cout << "Parsing input file\n");
  std::fstream infile(filename);
  std::string line;
  int vertexCount = 1;
  if (!infile.is_open()) {
    throw Exception("could not open file");
  }
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
    if (tokens[0] == "VERTEX") {
      auto type = getVertexType(tokens[2]);
      vertices.push_back(Vertex(vertexCount, type, tokens[3], tokens[5]));
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

int getVertexId(const std::vector<Vertex> vertices,
                const std::string &name,
                VertexType type) {
  auto pred = [&] (const Vertex &v) { return v.type == type &&
                                             v.name == name; };
  auto it = std::find_if(std::begin(vertices), std::end(vertices), pred);
  if (it != std::end(vertices)) {
    DEBUG(std::cout<<"Vertex "<<it->id<<" matches "<<name
                   <<" of type "<<getVertexTypeStr(type)<<"\n");
    return it->id;
  }
  return NULL_VERTEX_ID;
}

int getStartVertexId(const std::vector<Vertex> vertices,
                     const std::string &name) {
  if (int vId = getVertexId(vertices, name, REG_SRC))   return vId;
  if (int vId = getVertexId(vertices, name, VAR_INPUT)) return vId;
  if (int vId = getVertexId(vertices, name, VAR))       return vId;
  throw Exception(std::string("could not find start vertex ")+name);
}

int getEndVertexId(const std::vector<Vertex> vertices,
                     const std::string &name) {
  if (int vId = getVertexId(vertices, name, REG_DST))        return vId;
  if (int vId = getVertexId(vertices, name, REG_DST_OUTPUT)) return vId;
  if (int vId = getVertexId(vertices, name, VAR_OUTPUT))     return vId;
  if (int vId = getVertexId(vertices, name, VAR_INOUT))      return vId;
  if (int vId = getVertexId(vertices, name, VAR))            return vId;
  throw Exception(std::string("could not find end vertex ")+name);
}

int getMidVertexId(const std::vector<Vertex> vertices,
                   const std::string &name) {
  if (int vId = getVertexId(vertices, name, VAR))        return vId;
  if (int vId = getVertexId(vertices, name, VAR_WIRE))   return vId;
  if (int vId = getVertexId(vertices, name, VAR_INPUT))  return vId;
  if (int vId = getVertexId(vertices, name, VAR_OUTPUT)) return vId;
  if (int vId = getVertexId(vertices, name, VAR_INOUT))  return vId;
  throw Exception(std::string("could not find mid vertex ")+name);
}

void dumpPath(const std::vector<Vertex> vertices,
              const std::vector<int> path) {
  for (int vertexId : path) {
    if (vertices[vertexId-1].type != LOGIC) {
      std::cout << "  " << vertices[vertexId-1].name << "\n";
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

/// Build a Boost graph object.
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
  // Perform some checks.
  for (auto &vertex : vertices) {
    // Source registers don't have in edges.
    if (vertex.type == REG_SRC)
      if (boost::in_degree(boost::vertex(vertex.id, graph), graph) > 0)
         std::cout << "Warning: source reg " << vertex.name
                   << " (" << vertex.id << ") has in edges" << "\n";
    // Destination registers don't have out edges.
    if (vertex.type == REG_DST ||
        vertex.type == REG_DST_OUTPUT)
      if (boost::out_degree(boost::vertex(vertex.id, graph), graph) > 0)
        std::cout << "Warning: destination reg " << vertex.name
                  << " (" << vertex.id << ") has out edges"<<"\n";
  }
  return graph;
}

/// Dump a Graphviz dotfile of the netlist graph for visualisation.
void dumpDotFile(const std::vector<Vertex> &vertices,
                 const std::vector<Edge> &edges,
                 const std::string &outputFilename) {
  std::stringstream ss;
  ss << "digraph netlist_graph {\n";
  for (auto &vertex : vertices)
    ss << "  " << vertex.id << " [label=\""
       << getVertexTypeStr(vertex.type) << "\\n"
       << vertex.name << " (" << vertex.id << ")\"];\n";
  for (auto &edge : edges)
    ss << "  " << edge.src << " -> " << edge.dst << ";\n";
  ss << "}\n";
  // Write to file.
  std::ofstream outputFile(outputFilename);
  if (!outputFile.is_open())
    throw Exception(std::string("unable to open ")+outputFilename);
  outputFile << ss.str();
  outputFile.close();
  // Print command line to generate graph file.
  std::cout << "dot -Tpdf " << outputFilename << " -o graph.pdf\n";
}

/// Dump unique names of vars/regs/wires in the netlist for searching.
void dumpVertexNames(const std::vector<Vertex> vertices) {
  std::unordered_set<std::string> names;
  for (auto &vertex : vertices) {
    if (vertex.type == LOGIC)
      continue;
    names.insert(std::string(getVertexTypeStr(vertex.type))+" "+vertex.name);
  }
  std::vector<std::string> sortedNames(names.begin(), names.end());
  std::sort(sortedNames.begin(), sortedNames.end());
  for (auto &name : sortedNames) {
    std::cout << std::setw(8) << name << "\n";
  }
}

/// Pretty print a path (some sequence of vertices).
void printPathReport(const std::vector<Vertex> &vertices,
                     const std::vector<int> path,
                     bool netsOnly=false,
                     bool filenamesOnly=false) {
  // Determine the max length of a name.
  size_t maxNameLength = 0;
  for (auto &vertexId : path) {
    maxNameLength = std::max(maxNameLength, vertices[vertexId-1].name.size());
  }
  // Print each vertex on the path.
  for (auto &vertexId : path) {
    auto &vertex = vertices[vertexId-1];
    auto path = filenamesOnly ? fs::path(vertex.loc).filename()
                              : fs::path(vertex.loc);
    if (netsOnly) {
      if (vertex.type == LOGIC)
        continue;
      std::cout << "  " << std::left
                << std::setw(maxNameLength+1) << vertex.name
                << path.string() << "\n";
    } else {
      std::cout << "  " << std::left
                << std::setw(maxNameLength+1) << vertex.name
                << std::setw(8) << getVertexTypeStr(vertex.type) << " "
                << path.string() << "\n";
    }
  }
}

/// Use Verilator to compile a graph of the flattened Verilog netlist.
int compileGraph(const std::vector<std::string> &includes,
                 const std::vector<std::string> &defines,
                 const std::vector<std::string> &inputFiles,
                 const std::string &outputFile) {
  fs::path programLocation = boost::dll::program_location().parent_path();
  fs::path verilatorExe = programLocation / fs::path("verilator_bin");
  std::vector<std::string> args{"+1800-2012ext+.sv",
                                "--lint-only",
                                "--dump-netlist-graph",
                                "--error-limit", "10000",
                                "-o", outputFile};
  for (auto &path : includes)
    args.push_back(std::string("+incdir+")+path);
  for (auto &define : defines)
    args.push_back(std::string("-D")+define);
  for (auto &path : inputFiles)
    args.push_back(path);
  std::stringstream ss;
  for (auto &arg : args)
    ss << arg << " ";
  DEBUG(std::cout << "Running: " << verilatorExe << " " << ss.str() << "\n");
  return bp::system(verilatorExe, bp::args(args));
}

/// Report all paths fanning out from a net/register/port.
void reportAllFanout(const std::string &startName,
                     const std::vector<Vertex> &vertices,
                     Graph &graph,
                     bool netsOnly=false,
                     bool filenamesOnly=false) {
  int startVertexId = getStartVertexId(vertices, startName);
  DEBUG(std::cout << "Performing DFS from "
                  << vertices[startVertexId].name << "\n");
  ParentMap parentMap;
  boost::depth_first_search(graph,
      boost::visitor(DfsVisitor(parentMap, false))
        .root_vertex(startVertexId));
  // Check for a path between startPoint and each register.
  int pathCount = 0;
  for (auto &vertex : vertices) {
    if (vertex.type == REG_SRC ||
        vertex.type == REG_DST ||
        vertex.type == REG_DST_OUTPUT ||
        vertex.type == VAR_OUTPUT ||
        vertex.type == VAR_INOUT) {
      auto path = determinePath(parentMap, std::vector<int>(),
                                startVertexId, vertex.id);
      std::reverse(std::begin(path), std::end(path));
      if (!path.empty()) {
        std::cout << "Path " << ++pathCount << "\n";
        printPathReport(vertices, path, netsOnly, filenamesOnly);
        std::cout << "\n";
      }
    }
  }
  std::cout << "Found " << pathCount << " path(s)\n";
}

/// Report all paths fanning into a net/register/port.
void reportAllFanin(const std::string &endName,
                    const std::vector<Vertex> &vertices,
                    boost::reverse_graph<Graph> graph,
                    bool netsOnly=false,
                    bool filenamesOnly=false) {
  int endVertexId = getEndVertexId(vertices, endName);
  DEBUG(std::cout << "Performing DFS in reverse graph from "
                  << vertices[endVertexId].name << "\n");
  ParentMap parentMap;
  boost::depth_first_search(graph,
      boost::visitor(DfsVisitor(parentMap, false))
        .root_vertex(endVertexId));
  // Check for a path between endPoint and each register.
  int pathCount = 0;
  for (auto &vertex : vertices) {
    if (vertex.type == REG_SRC ||
        vertex.type == REG_DST ||
        vertex.type == REG_DST_OUTPUT ||
        vertex.type == VAR_INPUT ||
        vertex.type == VAR_INOUT) {
      auto path = determinePath(parentMap, std::vector<int>(),
                                endVertexId, vertex.id);
      if (!path.empty()) {
        std::cout << "Path " << ++pathCount << "\n";
        printPathReport(vertices, path, netsOnly, filenamesOnly);
        std::cout << "\n";
      }
    }
  }
  std::cout << "Found " << pathCount << " paths\n";
}

/// Report a single path between a set of named points.
void reportAnyPointToPoint(Graph &graph,
                           const std::vector<int> waypoints,
                           const std::vector<Vertex> vertices,
                           bool netsOnly=false,
                           bool filenamesOnly=false) {
  std::vector<int> path;
  // Construct the path between each adjacent waypoints.
  for (size_t i = 0; i < waypoints.size()-1; ++i) {
    int startVertexId = waypoints[i];
    int endVertexId = waypoints[i+1];
    DEBUG(std::cout << "Performing DFS from "
                    << vertices[startVertexId].name << "\n");
    ParentMap parentMap;
    boost::depth_first_search(graph,
        boost::visitor(DfsVisitor(parentMap, false))
          .root_vertex(startVertexId));
    DEBUG(std::cout << "Determining a path to " << endVertexId << "\n");
    auto subPath = determinePath(parentMap, std::vector<int>(),
                                 startVertexId, endVertexId);
    if (subPath.empty())
        throw Exception(std::string("no path from ")
                            +vertices[startVertexId].name+" to "
                            +vertices[endVertexId].name);
    std::reverse(std::begin(subPath), std::end(subPath));
    path.insert(std::end(path), std::begin(subPath), std::end(subPath)-1);
  }
  path.push_back(waypoints.back());
  printPathReport(vertices, path, netsOnly, filenamesOnly);
}

/// Report all paths between start and end points.
void reportAllPointToPoint(Graph &graph,
                           const std::vector<int> waypoints,
                           const std::vector<Vertex> vertices,
                           bool netsOnly=false,
                           bool filenamesOnly=false) {
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
    printPathReport(vertices, path, netsOnly, filenamesOnly);
  }
}

} // End anonymous namespace.

int main(int argc, char **argv) {
  try {
    // Command line options.
    std::vector<std::string> inputFiles;
    std::string outputFilename;
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
      ("help,h",        "Display help")
      ("start,s",       po::value<std::string>(&startName), "Start point")
      ("end,e",         po::value<std::string>(&endName),   "End point")
      ("through,t",     po::value<std::vector<std::string>>(&throughNames),
       "Through point")
      ("allpaths",      "Find all paths between two points (exponential time)")
      ("netsonly",      "Only display nets in path report")
      ("filenamesonly", "Only display filenames in path report")
      ("compile",       "Compile a netlist graph from Verilog source")
      ("include,I",     po::value<std::vector<std::string>>()->composing(),
                        "include path (only with --compile)")
      ("define,D",      po::value<std::vector<std::string>>()->composing(),
                        "define a preprocessor macro (only with --compile)")
      ("dotfile",       "Dump dotfile of netlist graph")
      ("dumpnames",     "Dump list of names in netlist")
      ("outfile,o",     po::value<std::string>(&outputFilename)
                          ->default_value(DEFAULT_OUTPUT_FILENAME),
                        "output file")
      ("debug",         "Print debugging information");
    allOptions.add(genericOptions).add(hiddenOptions);
    // Parse command line arguments.
    po::store(po::command_line_parser(argc, argv).
                  options(allOptions).positional(p).run(), vm);
    debugMode = vm.count("debug") > 0;
    bool displayHelp   = vm.count("help");
    bool dumpDotfile   = vm.count("dotfile");
    bool dumpNames     = vm.count("dumpnames");
    bool allPaths      = vm.count("allpaths");
    bool netsOnly      = vm.count("filenamesonly");
    bool filenamesOnly = vm.count("netsonly");
    bool compile       = vm.count("compile");
    if (displayHelp) {
      std::cout << "OVERVIEW: Query paths in a Verilog netlist\n\n";
      std::cout << "USAGE: " << argv[0] << " [options] infile\n\n";
      std::cout << genericOptions << "\n";
      return 1;
    }
    notify(vm);

    // Call Verilator to produce graph file.
    if (compile) {
      if (outputFilename == DEFAULT_OUTPUT_FILENAME)
         outputFilename += ".graph";
      auto includes = vm.count("include")
                        ? vm["include"].as<std::vector<std::string>>()
                        : std::vector<std::string>{};
      auto defines = vm.count("define")
                        ? vm["define"].as<std::vector<std::string>>()
                        : std::vector<std::string>{};
      return compileGraph(includes, defines, inputFiles, outputFilename);
    }

    // Parse the input file.
    if (inputFiles.size() > 1)
      throw Exception("multiple graph files specified");
    parseFile(inputFiles.front(), vertices, edges);

    // Dump dot file.
    if (dumpDotfile) {
      if (outputFilename == DEFAULT_OUTPUT_FILENAME)
         outputFilename += ".dot";
      dumpDotFile(vertices, edges, outputFilename);
      return 0;
    }

    // Dump netlist names.
    if (dumpNames) {
      dumpVertexNames(vertices);
      return 0;
    }

    // Build the graph.
    auto graph = buildGraph(vertices, edges);

    if (startName.empty() && endName.empty()) {
      throw Exception("no start and/or end point specified");
    }

    // Report paths fanning out from startName.
    if (!startName.empty() && endName.empty()) {
      if (!throughNames.empty())
        throw Exception("through points not supported for start only");
      reportAllFanout(startName, vertices, graph, netsOnly, filenamesOnly);
      return 0;
    }

    // Report paths fanning in to endName.
    if (startName.empty() && !endName.empty()) {
      if (!throughNames.empty())
        throw Exception("through points not supported for end only");
      reportAllFanin(endName, vertices, boost::make_reverse_graph(graph),
                     netsOnly, filenamesOnly);
      return 0;
    }

    // Find vertices in graph and compile path waypoints.
    waypoints.push_back(getStartVertexId(vertices, startName));
    for (auto &throughName : throughNames) {
      int vertexId = getMidVertexId(vertices, throughName);
      waypoints.push_back(vertexId);
    }
    // Look for a register end point, otherwise any matching variable.
    auto endVertexId = getEndVertexId(vertices, endName);
    waypoints.push_back(endVertexId);

    // Report all paths between two points.
    if (allPaths) {
      reportAllPointToPoint(graph, waypoints, vertices,
                            netsOnly, filenamesOnly);
      return 0;
    }

    // Report a paths between two points.
    reportAnyPointToPoint(graph, waypoints, vertices, filenamesOnly, netsOnly);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
