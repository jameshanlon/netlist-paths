#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <boost/dll.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/process.hpp>
#include <boost/python.hpp>
#include "netlist_paths/RunVerilator.hpp"
#include "netlist_paths/Options.hpp"

namespace bp = boost::process;

using namespace netlist_paths;

RunVerilator::RunVerilator() {
  Options::getInstance();
  // Locate the Verilator binary relative to this program.
  fs::path programLocation = boost::dll::program_location().parent_path();
  verilatorExe = programLocation / fs::path("np-verilator_bin");
}

RunVerilator::RunVerilator(const std::string &binLocation) {
  Options::getInstance();
  // Use a specific location to find Verilator.
  verilatorExe = fs::path(binLocation) / fs::path("np-verilator_bin");
}

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
  BOOST_LOG_TRIVIAL(info) << boost::format("Running %s %s") % verilatorExe % ss.str();
  return bp::system(verilatorExe, bp::args(args));
}

// A specialisation to be called in the Python wrapper
int RunVerilator::run(const boost::python::list &_includes,
                      const boost::python::list &_defines,
                      const boost::python::list &_inputFiles,
                      const std::string &outputFile) const {
    std::vector<std::string> includes, defines, inputFiles;
    for (int i = 0; i < len(_includes); i++) {
        includes.push_back(boost::python::extract<std::string>(_includes[i]));
    }
    for (int i = 0; i < len(_defines); i++) {
        defines.push_back(boost::python::extract<std::string>(_defines[i]));
    }
    for (int i = 0; i < len(_inputFiles); i++) {
        inputFiles.push_back(boost::python::extract<std::string>(_inputFiles[i]));
    }
    return run(includes, defines, inputFiles, outputFile);
}

/// A specialistion of run used for testing.
int RunVerilator::run(const std::string& inputFile, const std::string& outputFile) const {
  auto inputFiles = {inputFile};
  return run({}, {}, inputFiles, outputFile);
}
