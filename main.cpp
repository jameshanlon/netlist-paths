#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "netlist_paths/AnalyseGraph.hpp"
#include "netlist_paths/CompileGraph.hpp"
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/utilities.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

netlist_paths::Options options;

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
                          ->default_value(netlist_paths::DEFAULT_OUTPUT_FILENAME),
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

    netlist_paths::AnalyseGraph analyseGraph;
    std::vector<int> waypoints;

    // Call Verilator to produce graph file.
    if (options.compile) {
      if (outputFilename == netlist_paths::DEFAULT_OUTPUT_FILENAME)
         outputFilename += ".graph";
      auto includes = vm.count("include")
                        ? vm["include"].as<std::vector<std::string>>()
                        : std::vector<std::string>{};
      auto defines = vm.count("define")
                        ? vm["define"].as<std::vector<std::string>>()
                        : std::vector<std::string>{};
      netlist_paths::CompileGraph compileGraph;
      return compileGraph.run(includes,
                              defines,
                              inputFiles,
                              outputFilename);
    }

    // Parse the input file.
    if (inputFiles.size() > 1)
      throw netlist_paths::Exception("multiple graph files specified");
    analyseGraph.parseFile(inputFiles.front());

    // Dump dot file.
    if (options.dumpDotfile) {
      if (outputFilename == netlist_paths::DEFAULT_OUTPUT_FILENAME)
         outputFilename += ".dot";
      analyseGraph.dumpDotFile(outputFilename);
      return 0;
    }

    // Dump netlist names.
    if (options.dumpNames) {
      analyseGraph.dumpVertexNames();
      return 0;
    }

    if (startName.empty() && endName.empty()) {
      throw netlist_paths::Exception("no start and/or end point specified");
    }

    // Report paths fanning out from startName.
    if (!startName.empty() && endName.empty()) {
      if (!throughNames.empty())
        throw netlist_paths::Exception("through points not supported for start only");
      analyseGraph.reportAllFanout(startName);
      return 0;
    }

    // Report paths fanning in to endName.
    if (startName.empty() && !endName.empty()) {
      if (!throughNames.empty())
        throw netlist_paths::Exception("through points not supported for end only");
      analyseGraph.reportAllFanin(endName);
      return 0;
    }

    // Find vertices in graph and compile path waypoints.
    waypoints.push_back(analyseGraph.getStartVertexId(startName));
    for (auto &throughName : throughNames) {
      int vertexId = analyseGraph.getMidVertexId(throughName);
      waypoints.push_back(vertexId);
    }
    // Look for a register end point, otherwise any matching variable.
    auto endVertexId = analyseGraph.getEndVertexId(endName);
    waypoints.push_back(endVertexId);

    // Report all paths between two points.
    if (options.allPaths) {
      analyseGraph.reportAllPointToPoint(waypoints);
      return 0;
    }

    // Report a paths between two points.
    analyseGraph.reportAnyPointToPoint(waypoints);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
