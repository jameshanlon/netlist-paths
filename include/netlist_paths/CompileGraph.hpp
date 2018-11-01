#ifndef NETLIST_PATHS_COMPILE_GRAPH_HPP
#define NETLIST_PATHS_COMPILE_GRAPH_HPP

#include <vector>
#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace netlist_paths {

class CompileGraph {
private:
  fs::path verilatorExe;

public:
  CompileGraph();
  CompileGraph(const std::string &verilatorLocation);
  int run(const std::vector<std::string> &includes,
          const std::vector<std::string> &defines,
          const std::vector<std::string> &inputFiles,
          const std::string &outputFile) const;
};

} // End namespace.

#endif // NETLIST_PATHS_COMPILE_GRAPH_HPP

