#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
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
#define DEBUG(x) do { if (options.debugMode) { x; } } while (false)
#endif

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace bp = boost::process;

namespace {

using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
                                    boost::bidirectionalS>;
using ParentMap = std::map<int, std::vector<int>>;

const int NULL_VERTEX_ID = 0;
const int VERTEX_TYPE_STR_MAX_LEN = 16;
const char *DEFAULT_OUTPUT_FILENAME = "netlist";

struct Exception : public std::exception {
   std::string msg;
   Exception(std::string msg) : msg(msg) {}
   ~Exception() throw () {}
   const char* what() const throw() { return msg.c_str(); }
};

struct Options {
  bool debugMode;
  bool displayHelp;
  bool dumpDotfile;
  bool dumpNames;
  bool allPaths;
  bool netsOnly;
  bool filenamesOnly;
  bool compile;
  Options() :
      debugMode(false),
      displayHelp(false),
      dumpDotfile(false),
      dumpNames(false),
      allPaths(false),
      netsOnly(false),
      filenamesOnly(false),
      compile(false) {}
};

Options options;

enum class VertexType {
  NONE,
  LOGIC,
  ASSIGN,
  ASSIGNW,
  ALWAYS,
  INITIAL,
  REG_SRC,
  REG_DST,
  REG_DST_OUTPUT,
  VAR,
  VAR_WIRE,
  VAR_INPUT,
  VAR_OUTPUT,
  VAR_INOUT,
};

VertexType getVertexType(const std::string &type) {
       if (type == "LOGIC")          return VertexType::LOGIC;
  else if (type == "ASSIGN")         return VertexType::ASSIGN;
  else if (type == "ASSIGNW")        return VertexType::ASSIGNW;
  else if (type == "ALWAYS")         return VertexType::ALWAYS;
  else if (type == "INITIAL")        return VertexType::INITIAL;
  else if (type == "REG_SRC")        return VertexType::REG_SRC;
  else if (type == "REG_DST")        return VertexType::REG_DST;
  else if (type == "REG_DST_OUTPUT") return VertexType::REG_DST_OUTPUT;
  else if (type == "VAR")            return VertexType::VAR;
  else if (type == "VAR_WIRE")       return VertexType::VAR_WIRE;
  else if (type == "VAR_INPUT")      return VertexType::VAR_INPUT;
  else if (type == "VAR_OUTPUT")     return VertexType::VAR_OUTPUT;
  else if (type == "VAR_INOUT")      return VertexType::VAR_INOUT;
  else {
    throw Exception(std::string("unexpected vertex type: ")+type);
  }
}

const char *getVertexTypeStr(VertexType type) {
  switch (type) {
    case VertexType::LOGIC:          return "LOGIC";
    case VertexType::ASSIGN:         return "ASSIGN";
    case VertexType::ASSIGNW:        return "ASSIGNW";
    case VertexType::ALWAYS:         return "ALWAYS";
    case VertexType::INITIAL:        return "INITIAL";
    case VertexType::REG_SRC:        return "REG_SRC";
    case VertexType::REG_DST:        return "REG_DST";
    case VertexType::REG_DST_OUTPUT: return "REG_DST_OUTPUT";
    case VertexType::VAR:            return "VAR";
    case VertexType::VAR_WIRE:       return "VAR_WIRE";
    case VertexType::VAR_INPUT:      return "VAR_INPUT";
    case VertexType::VAR_OUTPUT:     return "VAR_OUTPUT";
    case VertexType::VAR_INOUT:      return "VAR_INOUT";
    default:                         return "UNKNOWN";
  }
}

struct Vertex {
  int id;
  VertexType type;
  std::string name;
  std::string loc;
  bool isTop;
  Vertex(int id, VertexType type) :
    id(id), type(type) {}
  Vertex(int id, VertexType type, const std::string &loc) :
    id(id), type(type), loc(loc) {}
  Vertex(int id,
         VertexType type,
         const std::string &name,
         const std::string &loc) :
      id(id), type(type), name(name), loc(loc) {
    // Check there are no Vlvbound nodes.
    assert(name.find("__Vlvbound") == std::string::npos);
    // module.name or name is top level, but module.submodule.name is not.
    std::vector<std::string> tokens;
    boost::split(tokens, name, boost::is_any_of("."));
    isTop = tokens.size() < 3;
  }
  bool isLogic() const {
    return type == VertexType::LOGIC ||
           type == VertexType::ASSIGN ||
           type == VertexType::ASSIGNW ||
           type == VertexType::ALWAYS ||
           type == VertexType::INITIAL;
  }
  bool isStartPoint() const {
    return type == VertexType::REG_SRC ||
           (type == VertexType::VAR_INPUT && isTop) ||
           (type == VertexType::VAR_INOUT && isTop);
  }
  bool isEndPoint() const {
    return type == VertexType::REG_DST ||
           type == VertexType::REG_DST_OUTPUT ||
           (type == VertexType::VAR_OUTPUT && isTop) ||
           (type == VertexType::VAR_INOUT && isTop);
  }
  bool isReg() const {
    return type == VertexType::REG_DST ||
           type == VertexType::REG_DST_OUTPUT ||
           type == VertexType::REG_SRC;
  }
  bool canIgnore() const {
    // Ignore variables Verilator has introduced.
    return name.find("__Vdly") != std::string::npos ||
           name.find("__Vcell") != std::string::npos;
  }
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
  if (!infile.is_open()) {
    throw Exception("could not open file");
  }
  while (std::getline(infile, line)) {
    std::vector<std::string> tokens;
    boost::split(tokens, line, boost::is_any_of(" "));
    if (tokens[0] == "VERTEX") {
      int id = std::atoi(tokens[1].c_str());
      if (id == 0) {
        throw Exception("vertex has NULL ID");
      }
      auto type = getVertexType(tokens[2]);
      if (tokens[3] == "@") {
        // Unnamed logic vertex.
        auto &loc = tokens[4];
        vertices.push_back(Vertex(id, type, loc));
      } else {
        // Named net/port/register.
        assert(tokens[4] == "@");
        auto &name = tokens[3];
        auto &loc = tokens[5];
        vertices.push_back(Vertex(id, type, name, loc));
      }
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
  if (int v = getVertexId(vertices, name, VertexType::REG_SRC))   return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR_INPUT)) return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR))       return v;
  throw Exception(std::string("could not find start vertex ")+name);
}

int getEndVertexId(const std::vector<Vertex> vertices,
                     const std::string &name) {
  if (int v = getVertexId(vertices, name, VertexType::REG_DST))        return v;
  if (int v = getVertexId(vertices, name, VertexType::REG_DST_OUTPUT)) return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR_OUTPUT))     return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR_INOUT))      return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR))            return v;
  throw Exception(std::string("could not find end vertex ")+name);
}

int getMidVertexId(const std::vector<Vertex> vertices,
                   const std::string &name) {
  if (int v = getVertexId(vertices, name, VertexType::VAR))        return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR_WIRE))   return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR_INPUT))  return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR_OUTPUT)) return v;
  if (int v = getVertexId(vertices, name, VertexType::VAR_INOUT))  return v;
  throw Exception(std::string("could not find mid vertex ")+name);
}

void dumpPath(const std::vector<Vertex> vertices,
              const std::vector<int> path) {
  for (int vertexId : path) {
    if (!vertices[vertexId-1].isLogic()) {
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
    if (vertex.type == VertexType::REG_SRC)
      if (boost::in_degree(boost::vertex(vertex.id, graph), graph) > 0)
         std::cout << "Warning: source reg " << vertex.name
                   << " (" << vertex.id << ") has in edges" << "\n";
    // Destination registers don't have out edges.
    if (vertex.type == VertexType::REG_DST ||
        vertex.type == VertexType::REG_DST_OUTPUT)
      if (boost::out_degree(boost::vertex(vertex.id, graph), graph) > 0)
        std::cout << "Warning: destination reg " << vertex.name
                  << " (" << vertex.id << ") has out edges"<<"\n";
    // NOTE: vertices may be incorrectly marked as reg if a field of a
    // structure has a delayed assignment to a field of it.
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
  DEBUG(std::cout << "dot -Tpdf " << outputFilename << " -o graph.pdf\n");
}

/// Dump unique names of vars/regs/wires in the netlist for searching.
void dumpVertexNames(const std::vector<Vertex> vertices) {
  std::unordered_set<std::string> names;
  for (auto &vertex : vertices) {
    if (vertex.isLogic())
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
                     const std::vector<int> path) {
  // Determine the max length of a name.
  size_t maxNameLength = 0;
  for (auto &vertexId : path) {
    if (vertices[vertexId-1].canIgnore())
      continue;
    maxNameLength = std::max(maxNameLength, vertices[vertexId-1].name.size());
  }
  // Print each vertex on the path.
  for (auto it = path.begin(); it != path.end(); ++it) {
    auto &vertex = vertices[*it-1];
    if (vertex.canIgnore())
      continue;
    auto path = options.filenamesOnly ? fs::path(vertex.loc).filename()
                                      : fs::path(vertex.loc);
    if (options.netsOnly) {
      if (!vertex.isLogic()) {
        std::cout << "  " << std::left
                  << std::setw(maxNameLength+1) << vertex.name
                  << path.string() << "\n";
      }
    } else {
      if (vertex.isLogic()) {
        std::cout << "  " << std::left
                  << std::setw(maxNameLength+1)
                  << getVertexTypeStr(vertex.type)
                  << std::setw(VERTEX_TYPE_STR_MAX_LEN)
                  << "LOGIC"
                  << path.string() << "\n";
      } else {
        std::cout << "  " << std::left
                  << std::setw(maxNameLength+1)
                  << vertex.name
                  << std::setw(VERTEX_TYPE_STR_MAX_LEN)
                  << getVertexTypeStr(vertex.type)
                  << path.string() << "\n";
      }
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
                     Graph &graph) {
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
    if (vertex.isEndPoint()) {
      auto path = determinePath(parentMap, std::vector<int>(),
                                startVertexId, vertex.id);
      std::reverse(std::begin(path), std::end(path));
      if (!path.empty()) {
        std::cout << "Path " << ++pathCount << "\n";
        printPathReport(vertices, path);
        std::cout << "\n";
      }
    }
  }
  std::cout << "Found " << pathCount << " path(s)\n";
}

/// Report all paths fanning into a net/register/port.
void reportAllFanin(const std::string &endName,
                    const std::vector<Vertex> &vertices,
                    boost::reverse_graph<Graph> graph) {
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
    if (vertex.isStartPoint()) {
      auto path = determinePath(parentMap, std::vector<int>(),
                                endVertexId, vertex.id);
      if (!path.empty()) {
        std::cout << "Path " << ++pathCount << "\n";
        printPathReport(vertices, path);
        std::cout << "\n";
      }
    }
  }
  std::cout << "Found " << pathCount << " paths\n";
}

/// Report a single path between a set of named points.
void reportAnyPointToPoint(Graph &graph,
                           const std::vector<int> waypoints,
                           const std::vector<Vertex> vertices) {
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
  printPathReport(vertices, path);
}

/// Report all paths between start and end points.
void reportAllPointToPoint(Graph &graph,
                           const std::vector<int> waypoints,
                           const std::vector<Vertex> vertices) {
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
    printPathReport(vertices, path);
  }
}

} // End anonymous namespace.

int main(int argc, char **argv) {
  try {
    // Command line options.
    po::options_description hiddenOptions("Positional options");
    po::options_description genericOptions("General options");
    po::options_description allOptions("All options");
    po::positional_options_description p;
    po::variables_map vm;
    std::vector<std::string> inputFiles;
    std::string outputFilename;
    std::string startName;
    std::string endName;
    std::vector<std::string> throughNames;
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
    options.debugMode     = vm.count("debug") > 0;
    options.displayHelp   = vm.count("help");
    options.dumpDotfile   = vm.count("dotfile");
    options.dumpNames     = vm.count("dumpnames");
    options.allPaths      = vm.count("allpaths");
    options.netsOnly      = vm.count("filenamesonly");
    options.filenamesOnly = vm.count("netsonly");
    options.compile       = vm.count("compile");
    if (options.displayHelp) {
      std::cout << "OVERVIEW: Query paths in a Verilog netlist\n\n";
      std::cout << "USAGE: " << argv[0] << " [options] infile\n\n";
      std::cout << genericOptions << "\n";
      return 1;
    }
    notify(vm);

    // Call Verilator to produce graph file.
    if (options.compile) {
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
    if (options.dumpDotfile) {
      if (outputFilename == DEFAULT_OUTPUT_FILENAME)
         outputFilename += ".dot";
      dumpDotFile(vertices, edges, outputFilename);
      return 0;
    }

    // Dump netlist names.
    if (options.dumpNames) {
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
      reportAllFanout(startName, vertices, graph);
      return 0;
    }

    // Report paths fanning in to endName.
    if (startName.empty() && !endName.empty()) {
      if (!throughNames.empty())
        throw Exception("through points not supported for end only");
      reportAllFanin(endName, vertices, boost::make_reverse_graph(graph));
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
    if (options.allPaths) {
      reportAllPointToPoint(graph, waypoints, vertices);
      return 0;
    }

    // Report a paths between two points.
    reportAnyPointToPoint(graph, waypoints, vertices);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
