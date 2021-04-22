#ifndef NETLIST_PATHS_RUN_VERILATOR_HPP
#define NETLIST_PATHS_RUN_VERILATOR_HPP

#include <vector>
#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace netlist_paths {

/// A class that provides facilities to run Verilator and produce XML netlists.
class RunVerilator {
private:
  fs::path verilatorExe;

public:

  /// Default constructor. Locate the Netlist Paths Verilator executable
  /// relative to the location of the Netlist Paths library.
  RunVerilator();

  /// Constructor, providing a path to a Verilator executable.
  ///
  /// \param verilatorLocation The location of the Netlists Paths Verilator
  ///        executable (np-verilator_bin).
  RunVerilator(const std::string &verilatorLocation);

  /// Run Verilator.
  ///
  /// \param includes   A vector of search paths for include files.
  /// \param defines    A vector of macro definitions.
  /// \param inputFiles A vector of source file paths.
  /// \param outputFile A path specifying an output file.
  int run(const std::vector<std::string> &includes,
          const std::vector<std::string> &defines,
          const std::vector<std::string> &inputFiles,
          const std::string &outputFile) const;

  /// Run Verilator with a single source file and no other options.
  ///
  /// \param inputFile  A source file path.
  /// \param outputFile A path specifying an output file.
  int run(const std::string& inputFile,
          const std::string& outputFile) const;
};

} // End namespace.

#endif // NETLIST_RUN_VERILATOR_HPP
