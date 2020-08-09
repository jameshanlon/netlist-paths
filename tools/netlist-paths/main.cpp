#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "netlist_paths/NetlistPaths.hpp"
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/Debug.hpp"
#include "netlist_paths/RunVerilator.hpp"

namespace po = boost::program_options;

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
      ("from",          po::value<std::string>(&startName)
                          ->value_name("name"),
                        "Start point")
      ("to",            po::value<std::string>(&endName)
                          ->value_name("name"),
                        "End point")
      ("through",       po::value<std::vector<std::string>>(&throughNames)
                          ->composing()
                          ->value_name("name"),
                        "Through point")
      ("allpaths",      "Find all paths between two points (exponential time)")
      ("startpoints",   "Only report start points")
      ("endpoints",     "Only report end points")
      ("fanout",        "Determine the fan out degree to bit end points")
      ("fanin",         "Determine the fan in degree from bit start points")
      ("reportlogic",   "Display logic in path report")
      ("filenames",     "Display full filenames in path report")
      ("compile",       "Compile a netlist graph from Verilog source")
      ("include,I",     po::value<std::vector<std::string>>()
                          ->composing()
                          ->value_name("path"),
                        "include path (only with --compile)")
      ("define,D",      po::value<std::vector<std::string>>()
                          ->composing()
                          ->value_name("path"),
                        "define a preprocessor macro (only with --compile)")
      ("dotfile",       "Dump dotfile of netlist graph")
      ("dumpnames",     "Dump list of names in netlist")
      ("outfile,o",     po::value<std::string>(&outputFilename)
                          ->default_value(netlist_paths::DEFAULT_OUTPUT_FILENAME)
                          ->value_name("filename"),
                        "output file")
      ("boostparser",   "Use the boost GraphViz parser")
      ("verbose,v",     "Print information")
      ("debug,d",       "Print debugging information");
    allOptions.add(genericOptions).add(hiddenOptions);
    // Parse command line arguments.
    po::store(po::command_line_parser(argc, argv).
                  options(allOptions).positional(p).run(), vm);
    netlist_paths::options.debugMode     = vm.count("debug") > 0;
    netlist_paths::options.verboseMode   = vm.count("verbose") > 0;
    netlist_paths::options.displayHelp   = vm.count("help") > 0;
    netlist_paths::options.dumpDotfile   = vm.count("dotfile") > 0;
    netlist_paths::options.dumpNames     = vm.count("dumpnames") > 0;
    netlist_paths::options.fanOutDegree  = vm.count("fanout") > 0;
    netlist_paths::options.fanInDegree   = vm.count("fanin") > 0;
    netlist_paths::options.allPaths      = vm.count("allpaths") > 0;
    netlist_paths::options.startPoints   = vm.count("startpoints") > 0;
    netlist_paths::options.endPoints     = vm.count("endpoints") > 0;
    netlist_paths::options.reportLogic   = vm.count("reportlogic") > 0;
    netlist_paths::options.fullFileNames = vm.count("filenames") > 0;
    netlist_paths::options.compile       = vm.count("compile") > 0;
    netlist_paths::options.boostParser   = vm.count("boostparser") > 0;
    if (netlist_paths::options.displayHelp) {
      std::cout << "OVERVIEW: Query paths in a Verilog netlist\n\n";
      std::cout << "USAGE: " << argv[0] << " [options] infile\n\n";
      std::cout << genericOptions << "\n";
      return 1;
    }
    notify(vm);

    // Call Verilator to produce graph file.
    if (netlist_paths::options.compile) {
      if (outputFilename == netlist_paths::DEFAULT_OUTPUT_FILENAME)
         outputFilename += ".graph";
      auto includes = vm.count("include")
                        ? vm["include"].as<std::vector<std::string>>()
                        : std::vector<std::string>{};
      auto defines = vm.count("define")
                        ? vm["define"].as<std::vector<std::string>>()
                        : std::vector<std::string>{};
      netlist_paths::RunVerilator compileGraph;
      return compileGraph.run(includes,
                              defines,
                              inputFiles,
                              outputFilename);
    }

    // Parse the input file.
    if (inputFiles.size() > 1) {
      throw netlist_paths::Exception("multiple XML files specified");
    }
    auto netlistPaths = netlist_paths::NetlistPaths(inputFiles.front());

    // Dump dot file.
    if (netlist_paths::options.dumpDotfile) {
      if (outputFilename == netlist_paths::DEFAULT_OUTPUT_FILENAME) {
         outputFilename += ".dot";
      }
      netlistPaths.dumpDotFile(outputFilename);
      return 0;
    }

//    // Dump netlist names.
//    if (netlist_paths::options.dumpNames) {
//      auto vertices = netlist.getNames();
//      netlist.printNames(vertices);
//      return 0;
//    }
//
//    return 0; // TEMPORARY EARLY EXIT
//
//    // A start or an endpoint must be specified.
//    if (startName.empty() && endName.empty()) {
//      throw netlist_paths::Exception("no start and/or end point specified");
//    }
//
//    // Start only.
//    if (!startName.empty() && endName.empty()) {
//      if (!throughNames.empty()) {
//        throw netlist_paths::Exception("through points not supported for start only");
//      }
//      if (netlist_paths::options.fanOutDegree) {
//        // Report the fan out degree from startName.
//        std::cout << netlist.getfanOutDegree(startName) << "\n";
//        return 0;
//      } else if (netlist_paths::options.endPoints) {
//        // Report the end points.
//        auto paths = netlist.getAllFanOut(startName);
//        netlist_paths::Path fanOutEndPoints;
//        for (auto &path : paths) {
//          fanOutEndPoints.push_back(path.back());
//        }
//        netlist.printPathReport(fanOutEndPoints);
//        return 0;
//      }
//      // Report paths fanning out from startName.
//      auto paths = netlist.getAllFanOut(startName);
//      netlist.printPathReport(paths);
//      return 0;
//    }
//
//    // End only.
//    if (startName.empty() && !endName.empty()) {
//      if (!throughNames.empty())
//        throw netlist_paths::Exception("through points not supported for end only");
//      if (netlist_paths::options.fanInDegree) {
//        // Report the fan in degree to endName.
//        std::cout << netlist.getFanInDegree(endName) << "\n";
//        return 0;
//      } else if (netlist_paths::options.startPoints) {
//        // Report the start points.
//        auto paths = netlist.getAllFanIn(endName);
//        for (auto &path : paths) {
//           const netlist_paths::Path newPath = {path.front()};
//           netlist.printPathReport(newPath);
//        }
//        return 0;
//      }
//      // Report paths fanning in to endName.
//      auto paths = netlist.getAllFanIn(endName);
//      netlist.printPathReport(paths);
//      return 0;
//    }
//
//    // Compile path waypoints.
//    netlist.addStartpoint(startName);
//    for (auto &throughName : throughNames) {
//      netlist.addWaypoint(throughName);
//    }
//    netlist.addEndpoint(endName);
//
//    // Report all paths between two points.
//    if (netlist_paths::options.allPaths) {
//      if (netlist.numWaypoints() > 2) {
//        throw netlist_paths::Exception("through points not supported for all paths");
//      }
//      auto paths = netlist.getAllPointToPoint();
//      netlist.printPathReport(paths);
//      return 0;
//    }
//
//    // Report a path between two points.
//    auto path = netlist.getAnyPointToPoint();
//    if (path.empty()) {
//      throw netlist_paths::Exception(std::string("no path between points"));
//    }
//    netlist.printPathReport(path);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
