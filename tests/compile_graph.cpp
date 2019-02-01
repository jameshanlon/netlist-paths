#define BOOST_TEST_MODULE compile_graph

#include <fstream>
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "netlist_paths/CompileGraph.hpp"
#include "netlist_paths/AnalyseGraph.hpp"
#include "netlist_paths/Options.hpp"
#include "tests/definitions.hpp"

namespace fs = boost::filesystem;

netlist_paths::Options options;

struct TestContext {
  TestContext() {}
  netlist_paths::AnalyseGraph analyseGraph;
  void compile(const std::string &inFilename) {
    std::vector<std::string> includes = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> inputFiles = {inFilename};
    netlist_paths::CompileGraph compileGraph(installPrefix);
    auto outTemp = fs::unique_path();
    compileGraph.run(includes,
                     defines,
                     inputFiles,
                     outTemp.native());
    analyseGraph.parseFile(outTemp.native());
    analyseGraph.mergeDuplicateVertices();
    analyseGraph.checkGraph();
    fs::remove(outTemp);
  }
};

BOOST_FIXTURE_TEST_CASE(adder, TestContext) {
  auto testPath = fs::path(testPrefix) / "adder.sv";
  BOOST_CHECK_NO_THROW(compile(testPath.string()));
  analyseGraph.addStartpoint("adder.i_a");
  analyseGraph.addEndpoint("adder.o_sum");
  auto path = analyseGraph.getAnyPointToPoint();
  BOOST_TEST(!path.empty());
}

BOOST_FIXTURE_TEST_CASE(mux2, TestContext) {
  auto testPath = fs::path(testPrefix) / "mux2.sv";
  BOOST_CHECK_NO_THROW(compile(testPath.string()));
}

BOOST_FIXTURE_TEST_CASE(fsm, TestContext) {
  auto testPath = fs::path(testPrefix) / "fsm.sv";
  BOOST_CHECK_NO_THROW(compile(testPath.string()));
}
