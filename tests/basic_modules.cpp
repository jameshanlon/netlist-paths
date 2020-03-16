#define BOOST_TEST_MODULE compile_graph

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <fstream>
#include <iostream>
#include <string>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "netlist_paths/CompileGraph.hpp"
#include "netlist_paths/Netlist.hpp"
#include "netlist_paths/Options.hpp"
#include "tests/definitions.hpp"

namespace fs = boost::filesystem;

netlist_paths::Options options;

struct TestContext {
  TestContext() {}
  netlist_paths::Netlist netlist;
  /// Compile a test and create a netlist object.
  void compile(const std::string &inFilename) {
    auto testPath = fs::path(testPrefix) / inFilename;
    std::vector<std::string> includes = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> inputFiles = {testPath.string()};
    netlist_paths::CompileGraph compileGraph(installPrefix);
    auto outTemp = fs::unique_path();
    compileGraph.run(includes,
         defines,
         inputFiles,
         outTemp.native());
    netlist.parseFile(outTemp.native());
    netlist.mergeDuplicateVertices();
    netlist.checkGraph();
    fs::remove(outTemp);
  }
  /// Check all names are unique.
  void uniqueNames() {
    auto vertices = netlist.getNames();
    std::vector<std::string> names;
    for (auto v : vertices)
      names.push_back(netlist.getVertexName(v));
    auto last = std::unique(std::begin(names), std::end(names));
    names.erase(last, std::end(names));
    BOOST_TEST(vertices.size() == names.size());
  }
  /// Check all names are qualified with the top module name.
  void qualifiedNames(const char *topName) {
    for (auto v : netlist.getNames()) {
      BOOST_TEST(boost::starts_with(netlist.getVertexName(v), topName));
    }
  }
  bool pathExists(const std::string &start,
                  const std::string &end) { return netlist.pathExists(start, end); }
};

BOOST_FIXTURE_TEST_CASE(verilator, TestContext) {
  // Check the Verilator binary exists.
  BOOST_ASSERT(boost::filesystem::exists(installPrefix));
}

BOOST_FIXTURE_TEST_CASE(adder, TestContext) {
  BOOST_CHECK_NO_THROW(compile("adder.sv"));
  uniqueNames();
  qualifiedNames("adder");
  // Check paths between all start and end points are reported.
  auto startPoints = {"adder.i_a", "adder.i_b"};
  auto endPoints = {"adder.o_sum", "adder.o_co"};
  for (auto s : startPoints) {
    for (auto e : endPoints) {
      BOOST_TEST(pathExists(s, e));
    }
  }
}

BOOST_FIXTURE_TEST_CASE(counter, TestContext) {
  BOOST_CHECK_NO_THROW(compile("counter.sv"));
  uniqueNames();
  qualifiedNames("counter");
  BOOST_TEST(pathExists("counter.i_clk", "counter.counter_q"));
  BOOST_TEST(pathExists("counter.i_rst", "counter.counter_q"));
  BOOST_TEST(pathExists("counter.counter_q", "counter.o_count"));
  BOOST_CHECK_THROW(pathExists("counter.counter_q", "counter.o_wrap"), netlist_paths::Exception);
  // TODO: check --from o_counter has no fan out paths
}

BOOST_FIXTURE_TEST_CASE(fsm, TestContext) {
  BOOST_CHECK_NO_THROW(compile("fsm.sv"));
  uniqueNames();
  qualifiedNames("fsm");
}

BOOST_FIXTURE_TEST_CASE(mux2, TestContext) {
  BOOST_CHECK_NO_THROW(compile("mux2.sv"));
  uniqueNames();
  qualifiedNames("mux2");
}

BOOST_FIXTURE_TEST_CASE(pipeline, TestContext) {
  BOOST_CHECK_NO_THROW(compile("pipeline.sv"));
  uniqueNames();
  qualifiedNames("pipeline");
  BOOST_TEST(pathExists("pipeline.g_pipestage[0].u_pipestage.data_q", "pipeline.g_pipestage[1].u_pipestage.data_q"));
  BOOST_TEST(pathExists("pipeline.g_pipestage[1].u_pipestage.data_q", "pipeline.g_pipestage[2].u_pipestage.data_q"));
  BOOST_TEST(pathExists("pipeline.g_pipestage[2].u_pipestage.data_q", "pipeline.g_pipestage[3].u_pipestage.data_q"));
  BOOST_TEST(pathExists("pipeline.g_pipestage[3].u_pipestage.data_q", "pipeline.g_pipestage[4].u_pipestage.data_q"));
  BOOST_TEST(pathExists("pipeline.g_pipestage[4].u_pipestage.data_q", "pipeline.g_pipestage[5].u_pipestage.data_q"));
  BOOST_TEST(pathExists("pipeline.g_pipestage[5].u_pipestage.data_q", "pipeline.g_pipestage[6].u_pipestage.data_q"));
  BOOST_TEST(pathExists("pipeline.g_pipestage[6].u_pipestage.data_q", "pipeline.g_pipestage[7].u_pipestage.data_q"));
}

