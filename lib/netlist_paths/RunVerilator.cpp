#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <boost/dll.hpp>
#include <boost/process.hpp>
#include "netlist_paths/RunVerilator.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/Debug.hpp"

namespace bp = boost::process;

using namespace netlist_paths;

RunVerilator::RunVerilator() {
  // Locate the Verilator binary relative to this program.
  fs::path programLocation = boost::dll::program_location().parent_path();
  verilatorExe = programLocation / fs::path("np-verilator_bin");
}

RunVerilator::RunVerilator(const std::string &binLocation) {
  // Use a specific location to find Verilator.
  verilatorExe = fs::path(binLocation) / fs::path("np-verilator_bin");
}

/// Use Verilator to compile a graph of the flattened Verilog netlist.
int RunVerilator::run(const std::vector<std::string> &includes,
                      const std::vector<std::string> &defines,
                      const std::vector<std::string> &inputFiles,
                      const std::string &outputFile) const {
  std::vector<std::string> args{"+1800-2012ext+.sv",
                                "--bbox-sys",
                                "--bbox-unsup",
                                "--xml-only",
                                "--flatten",
                                "--error-limit", "10000",
                                "--xml-output", outputFile};
  for (auto &path : includes) {
    args.push_back(std::string("+incdir+")+path);
  }
  for (auto &define : defines) {
    args.push_back(std::string("-D")+define);
  }
  for (auto &path : inputFiles) {
    args.push_back(path);
  }
  std::stringstream ss;
  for (auto &arg : args) {
    ss << arg << " ";
  }
  INFO(std::cout << "Running: " << verilatorExe << " " << ss.str() << "\n");
  return bp::system(verilatorExe, bp::args(args));
}

/// A specialistion of run used for testing.
int RunVerilator::run(const std::string& inputFile, const std::string& outputFile) const {
  auto inputFiles = {inputFile};
  return run({}, {}, inputFiles, outputFile);
}
