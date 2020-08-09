#ifndef NETLIST_PATHS_RUN_VERILATOR_HPP
#define NETLIST_PATHS_RUN_VERILATOR_HPP

#include <vector>
#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace netlist_paths {

class RunVerilator {
private:
  fs::path verilatorExe;

public:
  RunVerilator();
  RunVerilator(const std::string &verilatorLocation);
  int run(const std::vector<std::string> &includes,
          const std::vector<std::string> &defines,
          const std::vector<std::string> &inputFiles,
          const std::string &outputFile) const;
  int run(const std::string& inputFile,
          const std::string& outputFile) const;
};

} // End namespace.

#endif // NETLIST_RUN_VERILATOR_HPP
