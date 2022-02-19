
#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test reporting of the correct path components.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_components, TestContext);

/// Test when there are no paths.
BOOST_AUTO_TEST_CASE(path_none) {
  BOOST_CHECK_NO_THROW(compile("empty_module.sv"));
  // Any path.
  auto vertices = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
  BOOST_TEST(vertices.empty());
  // All paths.
  auto paths = np->getAllPaths(netlist_paths::Waypoints("in", "out"));
  BOOST_TEST(paths.empty());
}

BOOST_AUTO_TEST_CASE(path_query_basic_assign_chain) {
  BOOST_CHECK_NO_THROW(compile("basic_assign_chain.sv"));
  auto path = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
  BOOST_TEST(path.length() == 7);
  CHECK_VAR_REPORT(path.getVertex(0), "PORT", "logic", "in");
  CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(2), "VAR", "logic", "basic_assign_chain.a");
  CHECK_LOG_REPORT(path.getVertex(3), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(4), "VAR", "logic", "basic_assign_chain.b");
  CHECK_LOG_REPORT(path.getVertex(5), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(6), "PORT", "logic", "out");
}

BOOST_AUTO_TEST_CASE(path_query_basic_comb_chain) {
  BOOST_CHECK_NO_THROW(compile("basic_comb_chain.sv"));
  auto path = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
  BOOST_TEST(path.length() == 7);
  CHECK_VAR_REPORT(path.getVertex(0), "PORT", "logic", "in");
  CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(2), "VAR", "logic", "basic_comb_chain.a");
  CHECK_LOG_REPORT(path.getVertex(3), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(4), "VAR", "logic", "basic_comb_chain.b");
  CHECK_LOG_REPORT(path.getVertex(5), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(6), "PORT", "logic", "out");
}

BOOST_AUTO_TEST_CASE(path_query_basic_ff_chain) {
  BOOST_CHECK_NO_THROW(compile("basic_ff_chain.sv"));
  {
    // in -> a
    auto path = np->getAnyPath(netlist_paths::Waypoints("in", "basic_ff_chain.a"));
    BOOST_TEST(path.length() == 3);
    CHECK_VAR_REPORT(path.getVertex(0), "PORT", "logic", "in");
    CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN_DLY");
    CHECK_VAR_REPORT(path.getVertex(2), "DST_REG", "logic", "basic_ff_chain.a");
  }
  {
    // a -> b
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.a", "basic_ff_chain.b"));
    BOOST_TEST(path.length() == 3);
    CHECK_VAR_REPORT(path.getVertex(0), "SRC_REG", "logic", "basic_ff_chain.a");
    CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN_DLY");
    CHECK_VAR_REPORT(path.getVertex(2), "DST_REG", "logic", "basic_ff_chain.b");
  }
  {
    // b -> out
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.b", "out"));
    BOOST_TEST(path.length() == 3);
    CHECK_VAR_REPORT(path.getVertex(0), "SRC_REG", "logic", "basic_ff_chain.b");
    CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN");
    CHECK_VAR_REPORT(path.getVertex(2), "PORT", "logic", "out");
  }
}

BOOST_AUTO_TEST_CASE(path_query_pipeline_module) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv"));
  // NOTE: can differentiate between the generate instances of the pipeline.
  auto path = np->getAnyPath(netlist_paths::Waypoints("i_data", "pipeline_module.g_pipestage[0].u_pipestage.data_q"));
  BOOST_TEST(path.length() == 6);
  CHECK_VAR_REPORT(path.getVertex(0), "PORT", "[31:0] logic", "i_data");
  CHECK_VAR_REPORT(path.getVertex(1), "VAR", "[31:0] logic", "pipeline_module.g_pipestage[0].u_pipestage.i_data");
  CHECK_LOG_REPORT(path.getVertex(2), "ASSIGN_ALIAS");
  CHECK_VAR_REPORT(path.getVertex(3), "VAR", "[31:0] logic", "pipeline_module.__Vcellinp__g_pipestage[0].u_pipestage__i_data");
  CHECK_LOG_REPORT(path.getVertex(4), "ASSIGN_DLY");
  CHECK_VAR_REPORT(path.getVertex(5), "DST_REG", "[31:0] logic", "pipeline_module.g_pipestage[0].u_pipestage.data_q");
}

BOOST_AUTO_TEST_CASE(path_query_pipeline_loops) {
  BOOST_CHECK_NO_THROW(compile("pipeline_loops.sv"));
  // NOTE: cannot currently differentiate between elements of the data_q array.
  auto path = np->getAnyPath(netlist_paths::Waypoints("i_data", "pipeline_loops.data_q"));
  BOOST_TEST(path.length() == 3);
  CHECK_VAR_REPORT(path.getVertex(0), "PORT", "[31:0] logic", "i_data");
  CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN_DLY");
  CHECK_VAR_REPORT(path.getVertex(2), "DST_REG", "[31:0] logic [7:0]", "pipeline_loops.data_q");
}

BOOST_AUTO_TEST_CASE(path_query_pipeline_no_loops) {
  BOOST_CHECK_NO_THROW(compile("pipeline_no_loops.sv"));
  // NOTE: cannot currently differentiate between elements of the data_q array.
  auto path = np->getAnyPath(netlist_paths::Waypoints("pipeline_no_loops.data_q", "pipeline_no_loops.data_q"));
  BOOST_TEST(path.length() == 3);
  CHECK_VAR_REPORT(path.getVertex(0), "SRC_REG", "[31:0] logic [2:0]", "pipeline_no_loops.data_q");
  CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN_DLY");
  CHECK_VAR_REPORT(path.getVertex(2), "DST_REG", "[31:0] logic [2:0]", "pipeline_no_loops.data_q");
}

BOOST_AUTO_TEST_SUITE_END();
