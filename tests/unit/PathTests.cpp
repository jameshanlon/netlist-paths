#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

BOOST_FIXTURE_TEST_SUITE(paths, TestContext);

//===----------------------------------------------------------------------===//
// Test path exists
//===----------------------------------------------------------------------===//

BOOST_AUTO_TEST_CASE(path_exists_adder) {
  BOOST_CHECK_NO_THROW(compile("adder.sv"));
  // Check paths between all start and end points are reported.
  auto startPoints = {"adder.i_a", "adder.i_b"};
  auto endPoints = {"adder.o_sum", "adder.o_co"};
  for (auto s : startPoints) {
    for (auto e : endPoints) {
      BOOST_TEST(np->startpointExists(s));
      BOOST_TEST(np->endpointExists(e));
      BOOST_TEST(np->pathExists(netlist_paths::Waypoints(s, e)));
      BOOST_CHECK_THROW(np->pathExists(netlist_paths::Waypoints(e, s)),
                        netlist_paths::Exception);
    }
  }
}

BOOST_AUTO_TEST_CASE(path_exists_counter) {
  BOOST_CHECK_NO_THROW(compile("counter.sv"));
  // Valid paths.
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("counter.i_clk", "counter.counter_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("counter.i_rst", "counter.counter_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("counter.counter_q", "counter.o_count")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("counter.counter_q", "counter.o_wrap")));
  // Invalid paths (invalid start/end points).
  BOOST_CHECK_THROW(np->pathExists(netlist_paths::Waypoints("counter.o_count", "counter.counter_q")), netlist_paths::Exception);
  BOOST_CHECK_THROW(np->pathExists(netlist_paths::Waypoints("counter.count_q", "counter.i_clk")), netlist_paths::Exception);
  BOOST_CHECK_THROW(np->pathExists(netlist_paths::Waypoints("counter.count_q", "counter.i_rst")), netlist_paths::Exception);
  // Invalid paths (valid start/end points).
  BOOST_TEST(!np->pathExists(netlist_paths::Waypoints("counter.i_clk", "counter.o_count")));
  BOOST_TEST(!np->pathExists(netlist_paths::Waypoints("counter.i_clk", "counter.o_wrap")));
  BOOST_TEST(!np->pathExists(netlist_paths::Waypoints("counter.i_rst", "counter.o_count")));
  BOOST_TEST(!np->pathExists(netlist_paths::Waypoints("counter.i_rst", "counter.o_wrap")));
  // TODO: check --from o_counter has no fan out paths
}

BOOST_AUTO_TEST_CASE(path_exists_pipeline_module) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv"));
  // Check paths exist between all stages.
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[0].u_pipestage.data_q",
                                                     "pipeline_module.g_pipestage[1].u_pipestage.data_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[1].u_pipestage.data_q",
                                                     "pipeline_module.g_pipestage[2].u_pipestage.data_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[2].u_pipestage.data_q",
                                                     "pipeline_module.g_pipestage[3].u_pipestage.data_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[3].u_pipestage.data_q",
                                                     "pipeline_module.g_pipestage[4].u_pipestage.data_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[4].u_pipestage.data_q",
                                                     "pipeline_module.g_pipestage[5].u_pipestage.data_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[5].u_pipestage.data_q",
                                                     "pipeline_module.g_pipestage[6].u_pipestage.data_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[6].u_pipestage.data_q",
                                                     "pipeline_module.g_pipestage[7].u_pipestage.data_q")));

  // Check multiple matching start/end points raise exception.
  netlist_paths::Options::getInstance().setMatchWildcard();
  netlist_paths::Options::getInstance().setMatchOneVertex();
  BOOST_CHECK_THROW(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[?].u_pipestage.data_q",
                                                            "pipeline_module.g_pipestage[1].u_pipestage.data_q")),
                    netlist_paths::Exception);
  BOOST_CHECK_THROW(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[0].u_pipestage.data_q",
                                                            "pipeline_module.g_pipestage[?].u_pipestage.data_q")),
                    netlist_paths::Exception);
  BOOST_CHECK_THROW(np->pathExists(netlist_paths::Waypoints("pipeline_module.g_pipestage[?].u_pipestage.data_q",
                                                            "pipeline_module.g_pipestage[?].u_pipestage.data_q")),
                    netlist_paths::Exception);
}

//===----------------------------------------------------------------------===//
// Test reporting of the correct path components.
//===----------------------------------------------------------------------===//

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
  CHECK_VAR_REPORT(path.getVertex(0), "VAR", "logic", "in");
  CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(2), "VAR", "logic", "basic_assign_chain.a");
  CHECK_LOG_REPORT(path.getVertex(3), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(4), "VAR", "logic", "basic_assign_chain.b");
  CHECK_LOG_REPORT(path.getVertex(5), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(6), "VAR", "logic", "out");
}

BOOST_AUTO_TEST_CASE(path_query_basic_comb_chain) {
  BOOST_CHECK_NO_THROW(compile("basic_comb_chain.sv"));
  auto path = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
  BOOST_TEST(path.length() == 7);
  CHECK_VAR_REPORT(path.getVertex(0), "VAR", "logic", "in");
  CHECK_LOG_REPORT(path.getVertex(1), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(2), "VAR", "logic", "basic_comb_chain.a");
  CHECK_LOG_REPORT(path.getVertex(3), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(4), "VAR", "logic", "basic_comb_chain.b");
  CHECK_LOG_REPORT(path.getVertex(5), "ASSIGN");
  CHECK_VAR_REPORT(path.getVertex(6), "VAR", "logic", "out");
}

BOOST_AUTO_TEST_CASE(path_query_basic_ff_chain) {
  BOOST_CHECK_NO_THROW(compile("basic_ff_chain.sv"));
  {
    // in -> a
    auto path = np->getAnyPath(netlist_paths::Waypoints("in", "basic_ff_chain.a"));
    BOOST_TEST(path.length() == 3);
    CHECK_VAR_REPORT(path.getVertex(0), "VAR", "logic", "in");
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
    CHECK_VAR_REPORT(path.getVertex(2), "VAR", "logic", "out");
  }
}

BOOST_AUTO_TEST_CASE(path_query_pipeline_module) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv"));
  // NOTE: can differentiate between the generate instances of the pipeline.
  auto path = np->getAnyPath(netlist_paths::Waypoints("i_data", "pipeline_module.g_pipestage[0].u_pipestage.data_q"));
  BOOST_TEST(path.length() == 6);
  CHECK_VAR_REPORT(path.getVertex(0), "VAR", "[31:0] logic", "i_data");
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
  CHECK_VAR_REPORT(path.getVertex(0), "VAR", "[31:0] logic", "i_data");
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

//===----------------------------------------------------------------------===//
// Test all paths query.
//===----------------------------------------------------------------------===//

BOOST_AUTO_TEST_CASE(path_all_paths) {
  BOOST_CHECK_NO_THROW(compile("multiple_paths.sv"));
  auto paths = np->getAllPaths(netlist_paths::Waypoints("in", "out"));
  BOOST_TEST(paths.size() == 3);
  // Path 1
  BOOST_TEST(paths[0].length() == 5);
  CHECK_VAR_REPORT(paths[0].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[0].getVertex(2), "VAR", "logic", "multiple_paths.a");
  CHECK_VAR_REPORT(paths[0].getVertex(4), "VAR", "logic", "out");
  // Path 2
  BOOST_TEST(paths[1].length() == 5);
  CHECK_VAR_REPORT(paths[1].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "VAR", "logic", "multiple_paths.b");
  CHECK_VAR_REPORT(paths[1].getVertex(4), "VAR", "logic", "out");
  // Path 3
  BOOST_TEST(paths[2].length() == 5);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[2].getVertex(2), "VAR", "logic", "multiple_paths.c");
  CHECK_VAR_REPORT(paths[2].getVertex(4), "VAR", "logic", "out");
}

//===----------------------------------------------------------------------===//
// Test fan in/out
//===----------------------------------------------------------------------===//

/// Test empty fan in and out paths.
BOOST_AUTO_TEST_CASE(path_fan_out_empty) {
  BOOST_CHECK_NO_THROW(compile("empty_module.sv"));
  BOOST_TEST(np->getAllFanOut("in").empty());
  BOOST_TEST(np->getAllFanIn("out").empty());
}

/// Test paths fanning out to three end points.
BOOST_AUTO_TEST_CASE(path_fan_out) {
  BOOST_CHECK_NO_THROW(compile("fan_out_in.sv"));
  auto paths = np->getAllFanOut("in");
  BOOST_TEST(paths.size() == 3);
  // Path 1
  BOOST_TEST(paths[0].length() == 3);
  CHECK_VAR_REPORT(paths[0].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[0].getVertex(2), "DST_REG", "logic", "fan_out_in.a");
  // Path 2
  BOOST_TEST(paths[1].length() == 3);
  CHECK_VAR_REPORT(paths[1].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "DST_REG", "logic", "fan_out_in.b");
  // Path 3
  BOOST_TEST(paths[2].length() == 3);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[2].getVertex(2), "DST_REG", "logic", "fan_out_in.c");
}

/// Test paths fanning into an end point.
BOOST_AUTO_TEST_CASE(path_fan_in) {
  BOOST_CHECK_NO_THROW(compile("fan_out_in.sv"));
  auto paths = np->getAllFanIn("out");
  BOOST_TEST(paths.size() == 3);
  // Path 1
  BOOST_TEST(paths[0].length() == 3);
  CHECK_VAR_REPORT(paths[0].getVertex(0), "SRC_REG", "logic", "fan_out_in.a");
  CHECK_VAR_REPORT(paths[0].getVertex(2), "VAR", "logic", "out");
  // Path 2
  BOOST_TEST(paths[1].length() == 3);
  CHECK_VAR_REPORT(paths[1].getVertex(0), "SRC_REG", "logic", "fan_out_in.b");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "VAR", "logic", "out");
  // Path 3
  BOOST_TEST(paths[2].length() == 3);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "SRC_REG", "logic", "fan_out_in.c");
  CHECK_VAR_REPORT(paths[2].getVertex(2), "VAR", "logic", "out");
}

/// Test that invalid through points throw exceptions.
BOOST_AUTO_TEST_CASE(path_fan_out_exceptions) {
  BOOST_CHECK_NO_THROW(compile("fan_out_in.sv"));
  // Fan out.
  // Invalid vertex.
  BOOST_CHECK_THROW(np->getAllFanOut("foo"), netlist_paths::Exception);
  // Not a start point.
  BOOST_CHECK_THROW(np->getAllFanOut("out"), netlist_paths::Exception);
  // Fan in.
  // Invalid vertex.
  BOOST_CHECK_THROW(np->getAllFanIn("foo"), netlist_paths::Exception);
  // Not a start point.
  BOOST_CHECK_THROW(np->getAllFanIn("in"), netlist_paths::Exception);
}

BOOST_AUTO_TEST_CASE(path_fan_out_modules) {
  // Test fan out paths ending in sub modules.
  BOOST_CHECK_NO_THROW(compile("fan_out_in_modules.sv"));
  auto paths = np->getAllFanOut("in");
  BOOST_TEST(paths.size() == 3);
  // Path 1
  BOOST_TEST(paths[0].length() == 3);
  CHECK_VAR_REPORT(paths[0].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[0].getVertex(2), "DST_REG", "logic", "fan_out_in_modules.foo_a.x");
  // Path 2
  BOOST_TEST(paths[1].length() == 3);
  CHECK_VAR_REPORT(paths[1].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "DST_REG", "logic", "fan_out_in_modules.foo_b.x");
  // Path 3
  BOOST_TEST(paths[2].length() == 3);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "VAR", "logic", "in");
  CHECK_VAR_REPORT(paths[2].getVertex(2), "DST_REG", "logic", "fan_out_in_modules.foo_c.x");
}

BOOST_AUTO_TEST_CASE(path_fan_in_modules) {
  // Test fan out paths starting in sub modules.
  BOOST_CHECK_NO_THROW(compile("fan_out_in_modules.sv"));
  auto paths = np->getAllFanIn("out");
  BOOST_TEST(paths.size() == 3);
  BOOST_TEST(paths[0].length() == 6);
  CHECK_VAR_REPORT(paths[0].getVertex(0), "SRC_REG", "logic", "fan_out_in_modules.foo_a.x");
  CHECK_VAR_REPORT(paths[0].getVertex(2), "VAR", "logic", "fan_out_in_modules.a");
  CHECK_VAR_REPORT(paths[0].getVertex(4), "VAR", "logic", "fan_out_in_modules.foo_a.out");
  CHECK_VAR_REPORT(paths[0].getVertex(5), "VAR", "logic", "out");
  BOOST_TEST(paths[1].length() == 6);\
  CHECK_VAR_REPORT(paths[1].getVertex(0), "SRC_REG", "logic", "fan_out_in_modules.foo_b.x");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "VAR", "logic", "fan_out_in_modules.b");
  CHECK_VAR_REPORT(paths[1].getVertex(4), "VAR", "logic", "fan_out_in_modules.foo_b.out");
  CHECK_VAR_REPORT(paths[1].getVertex(5), "VAR", "logic", "out");
  BOOST_TEST(paths[2].length() == 6);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "SRC_REG", "logic", "fan_out_in_modules.foo_c.x");
  CHECK_VAR_REPORT(paths[2].getVertex(2), "VAR", "logic", "fan_out_in_modules.c");
  CHECK_VAR_REPORT(paths[2].getVertex(4), "VAR", "logic", "fan_out_in_modules.foo_c.out");
  CHECK_VAR_REPORT(paths[2].getVertex(5), "VAR", "logic", "out");
}

BOOST_AUTO_TEST_CASE(path_false_fan_in_out) {
  // Demonstrate the false connections though an array variable used in a generate statement.
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv"));
  {
    auto paths = np->getAllFanOut("pipeline_module.g_pipestage[6].u_pipestage.data_q");
    BOOST_TEST(paths.size() == 10);
  }
  {
    auto paths = np->getAllFanIn("pipeline_module.g_pipestage[6].u_pipestage.data_q");
    BOOST_TEST(paths.size() == 14);
  }
}

//===----------------------------------------------------------------------===//
// Test through points.
//===----------------------------------------------------------------------===//

/// All paths: test querying paths when a single through point is provided.
BOOST_AUTO_TEST_CASE(all_paths_single_through_point) {
  BOOST_CHECK_NO_THROW(compile("multiple_paths.sv"));
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.a");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.size() == 1);
    BOOST_TEST(paths.front().getVertex(2)->getName() == "multiple_paths.a");
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.b");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.size() == 1);
    BOOST_TEST(paths.front().getVertex(2)->getName() == "multiple_paths.b");
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.c");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.size() == 1);
    BOOST_TEST(paths.front().getVertex(2)->getName() == "multiple_paths.c");
  }
  {
    // No valid path (no route between a and b).
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.a");
    waypoints.addThroughPoint("multiple_paths.b");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.empty());
  }
}

/// All paths: test querying paths when multiple through points are provided.
BOOST_AUTO_TEST_CASE(all_paths_multiple_through_points) {
  BOOST_CHECK_NO_THROW(compile("three_multi_path_stages.sv"));
  {
    // Check the number of paths without through points.
    auto waypoints = netlist_paths::Waypoints("in", "out");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.size() == 27);
  }
  {
    // Add the two intermediate fan-in/out points.
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("three_multi_path_stages.all_a");
    waypoints.addThroughPoint("three_multi_path_stages.all_b");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.size() == 27);
  }
}

/// Any path: test querying paths when a single through point is provided.
BOOST_AUTO_TEST_CASE(any_path_single_through_point) {
  BOOST_CHECK_NO_THROW(compile("multiple_paths.sv"));
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.a");
    auto path = np->getAnyPath(waypoints);
    BOOST_TEST(path.length() == 5);
    BOOST_TEST(path.getVertex(2)->getName() == "multiple_paths.a");
  }
  {
    // No valid path (no route between a and b).
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.a");
    waypoints.addThroughPoint("multiple_paths.b");
    auto path = np->getAnyPath(waypoints);
    BOOST_TEST(path.empty());
  }
}

/// Any path: test querying paths when multiple through points are provided.
BOOST_AUTO_TEST_CASE(any_path_multiple_through_points) {
  BOOST_CHECK_NO_THROW(compile("three_multi_path_stages.sv"));
  {
    // Add the two intermediate fan-in/out points.
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("three_multi_path_stages.all_a");
    waypoints.addThroughPoint("three_multi_path_stages.all_b");
    auto path = np->getAnyPath(waypoints);
    BOOST_TEST(path.length() == 13);
  }
  {
    // No valid path (ordering of through points).
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("three_multi_path_stages.all_b");
    waypoints.addThroughPoint("three_multi_path_stages.all_a");
    auto vertices = np->getAnyPath(waypoints);
    BOOST_TEST(vertices.empty());
  }
}

// Test that invalid through points throw exceptions.
BOOST_AUTO_TEST_CASE(through_point_exception) {
  BOOST_CHECK_NO_THROW(compile("multiple_paths.sv"));
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.foo"); // Doesn't exist.
    BOOST_CHECK_THROW(np->getAllPaths(waypoints), netlist_paths::Exception);
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("in"); // Not a valid through point.
    BOOST_CHECK_THROW(np->getAllPaths(waypoints), netlist_paths::Exception);
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("out"); // Not a valid through point.
    BOOST_CHECK_THROW(np->getAllPaths(waypoints), netlist_paths::Exception);
  }
}

//===----------------------------------------------------------------------===//
// Test avoid points.
//===----------------------------------------------------------------------===//

BOOST_AUTO_TEST_CASE(one_avoid_point) {
  // Test that avoiding a net prevents a path from being found.
  BOOST_CHECK_NO_THROW(compile("one_avoid_point.sv"));
  {
    auto waypoints = netlist_paths::Waypoints("i_a", "o_a");
    auto path = np->getAnyPath(waypoints);
    BOOST_TEST(!path.empty());
  }
  {
    auto waypoints = netlist_paths::Waypoints("i_a", "o_a");
    waypoints.addAvoidPoint("one_avoid_point.foo");
    auto path = np->getAnyPath(waypoints);
    BOOST_TEST(path.empty());
  }
  {
    auto waypoints = netlist_paths::Waypoints("i_a", "o_a");
    waypoints.addAvoidPoint("one_avoid_point.foo");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.empty());
  }
}

BOOST_AUTO_TEST_CASE(multiple_avoid_points) {
  // Test that avoiding combinations of alternative paths results in the correct
  // paths being found.
  BOOST_CHECK_NO_THROW(compile("multiple_paths.sv"));
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.size() == 3);
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addAvoidPoint("multiple_paths.a");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.size() == 2);
    BOOST_TEST(paths[0].getVertex(3)->getName() != "multiple_paths.a");
    BOOST_TEST(paths[1].getVertex(3)->getName() != "multiple_paths.a");
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addAvoidPoint("multiple_paths.a");
    waypoints.addAvoidPoint("multiple_paths.b");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.size() == 1);
    BOOST_TEST(paths[0].getVertex(3)->getName() != "multiple_paths.a");
    BOOST_TEST(paths[0].getVertex(3)->getName() != "multiple_paths.b");
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addAvoidPoint("multiple_paths.a");
    waypoints.addAvoidPoint("multiple_paths.b");
    waypoints.addAvoidPoint("multiple_paths.c");
    auto paths = np->getAllPaths(waypoints);
    BOOST_TEST(paths.empty());
  }
}

BOOST_AUTO_TEST_CASE(avoid_point_exception) {
  // Test that an invalid avoid point throws an exception.
  BOOST_CHECK_NO_THROW(compile("multiple_paths.sv"));
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addAvoidPoint("multiple_paths.foo"); // Doesn't exist.
    BOOST_CHECK_THROW(np->getAllPaths(waypoints), netlist_paths::Exception);
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.in"); // Not a valid avoid point.
    BOOST_CHECK_THROW(np->getAllPaths(waypoints), netlist_paths::Exception);
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("multiple_paths.out"); // Not a valid avoid point.
    BOOST_CHECK_THROW(np->getAllPaths(waypoints), netlist_paths::Exception);
  }
}

//===----------------------------------------------------------------------===//
// Test matching any start/finish points.
//===----------------------------------------------------------------------===//

BOOST_AUTO_TEST_CASE(multiple_separate_paths) {
  BOOST_CHECK_NO_THROW(compile("multiple_separate_paths.sv"));
  netlist_paths::Options::getInstance().setMatchWildcard();
  netlist_paths::Options::getInstance().setMatchAnyVertex();
  {
    auto path = np->getAnyPath(netlist_paths::Waypoints("i_*", "o_*"));
    BOOST_TEST(path.length());
  }
  {
    auto path = np->getAnyPath(netlist_paths::Waypoints("i_*", "o_a"));
    BOOST_TEST(path.length());
  }
  {
    auto path = np->getAnyPath(netlist_paths::Waypoints("i_a", "o_*"));
    BOOST_TEST(path.length());
  }
  {
    auto path = np->getAnyPath(netlist_paths::Waypoints("i_b", "o_b"));
    BOOST_TEST(path.length());
  }
}

//===----------------------------------------------------------------------===//
// Test matching of paths through modules with registered outputs (a registered
// output is a port and register). Because of the way that Verilator inlines the
// modules, the source names of the resisters are not preserved. However, there
// should be the correct number of registers and paths.
//===----------------------------------------------------------------------===//

BOOST_AUTO_TEST_CASE(paths_with_port_registers) {

  BOOST_CHECK_NO_THROW(compile("registered_output_path.sv"));

  // There are three registers.
  BOOST_TEST(np->getRegVerticesPtr().size() == 3);

  // Check that REG_ALIASES match against 'is register' queries.
  BOOST_TEST(np->regExists("registered_output_path.u_foo1.o_b"));
  BOOST_TEST(np->regExists("registered_output_path.u_foo2.o_b"));
  //BOOST_TEST(np->regExists("registered_output_path.u_foo3.o_b")); // Should be a REG_ALIAS
  BOOST_TEST(np->anyRegExists("registered_output_path.u_foo1.o_b"));
  BOOST_TEST(np->anyRegExists("registered_output_path.u_foo2.o_b"));
  //BOOST_TEST(np->anyRegExists("registered_output_path.u_foo3.o_b")); // Should be a REG_ALIAS

  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("in",
                                                     "registered_output_path.u_foo1.o_b")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("registered_output_path.u_foo1.o_b",
                                                     "registered_output_path.u_foo2.o_b")));
  // FIXME: u_foo3.o_b not marked as register, needs investigation.
  //BOOST_TEST(np->pathExists(netlist_paths::Waypoints("registered_output_path.u_foo2.o_b",
  //                                                   "registered_output_path.u_foo3.o_b")));
  //BOOST_TEST(np->pathExists(netlist_paths::Waypoints("registered_output_path.u_foo3.o_b",
  //                                                   "out")));

  // Fan out from source registers to a destination register plus two aliases.
  BOOST_TEST(np->getAllFanOut("in").size() == 3);
  BOOST_TEST(np->getAllFanOut("registered_output_path.data1").size() == 3);
  BOOST_TEST(np->getAllFanOut("registered_output_path.data2").size() == 1);
}

//===----------------------------------------------------------------------===//
// Test path queries through registers.
//===----------------------------------------------------------------------===//

BOOST_AUTO_TEST_CASE(through_registers) {
  BOOST_CHECK_NO_THROW(compile("basic_ff_chain.sv"));

  // Paths through regisers.
  {
    // Any path: in -> a -> b -> out
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto path = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
    BOOST_TEST(path.length() == 7);
    CHECK_VAR_REPORT(path.getVertex(0), "VAR", "logic", "in");
    CHECK_VAR_REPORT(path.getVertex(2), "DST_REG", "logic", "basic_ff_chain.a");
    CHECK_VAR_REPORT(path.getVertex(4), "DST_REG", "logic", "basic_ff_chain.b");
    CHECK_VAR_REPORT(path.getVertex(6), "VAR", "logic", "out");
  }
  {
    // Any path: a -> b -> out
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.a", "out"));
    BOOST_TEST(path.length() == 5);
    CHECK_VAR_REPORT(path.getVertex(0), "SRC_REG", "logic", "basic_ff_chain.a");
    CHECK_VAR_REPORT(path.getVertex(2), "DST_REG", "logic", "basic_ff_chain.b");
    CHECK_VAR_REPORT(path.getVertex(4), "VAR", "logic", "out");
  }
  {
    // Any path: b -> out
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.b", "out"));
    BOOST_TEST(path.length() == 3);
    CHECK_VAR_REPORT(path.getVertex(0), "SRC_REG", "logic", "basic_ff_chain.b");
    CHECK_VAR_REPORT(path.getVertex(2), "VAR", "logic", "out");
  }

  // Check also the through-register paths don't appear when not traversing registers.
  {
    netlist_paths::Options::getInstance().setTraverseRegisters(false);
    auto path = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
    BOOST_TEST(path.empty());
  }
  {
    netlist_paths::Options::getInstance().setTraverseRegisters(false);
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.a", "out"));
    BOOST_TEST(path.empty());
  }
  {
    netlist_paths::Options::getInstance().setTraverseRegisters(false);
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.a", "out"));
    BOOST_TEST(path.empty());
  }

  // Check fanout and fanin path queries.
  {
    // Fanout: in
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto paths = np->getAllFanOut("in");
    std::vector<std::string> endPoints = {"out", "basic_ff_chain.out", "basic_ff_chain.a", "basic_ff_chain.b"};
    BOOST_TEST(paths.size() == endPoints.size());
    for (auto path : paths) {
      BOOST_TEST((std::find(endPoints.begin(), endPoints.end(), path.getFinishVertex()->getName()) != endPoints.end()));
    }
  }
  {
    // Fanout: out
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto paths = np->getAllFanIn("out");
    std::vector<std::string> startPoints = {"in", "basic_ff_chain.in", "basic_ff_chain.a", "basic_ff_chain.b"};
    BOOST_TEST(paths.size() == startPoints.size());
    for (auto path : paths) {
      BOOST_TEST((std::find(startPoints.begin(), startPoints.end(), path.getStartVertex()->getName()) != startPoints.end()));
    }
  }
}

//===----------------------------------------------------------------------===//
// Test handling of aliases and path start/end point restrictions.
//===----------------------------------------------------------------------===//

// Alias start point (due to inlining of the sub module).
BOOST_AUTO_TEST_CASE(alias_start_point) {
  BOOST_CHECK_NO_THROW(compile("alias_start_point.sv"));
  netlist_paths::Options::getInstance().setRestrictStartPoints(false);
  // alias_start_point.u_a.out is an alias of alias_start_point.x, check that it can be used as a start point.
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("alias_start_point.x",       "alias_start_point.y_q")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("alias_start_point.u_a.out", "alias_start_point.y_q")));
}

// Check that paths between variables in two sub modules can start and end on
// the different variables or aliases of those points.
BOOST_AUTO_TEST_CASE(aliases_sub_comb) {
  BOOST_CHECK_NO_THROW(compile("aliases_sub_comb.sv"));
  netlist_paths::Options::getInstance().setRestrictStartPoints(false);
  netlist_paths::Options::getInstance().setRestrictEndPoints(false);
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_comb.u_a.out",       "aliases_sub_comb.u_b.in")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_comb.u_a.out",       "aliases_sub_comb.u_b.client_out")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_comb.u_a.client_in", "aliases_sub_comb.u_b.in")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_comb.u_a.client_in", "aliases_sub_comb.u_b.client_out")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_comb.u_b.out",       "aliases_sub_comb.u_a.in")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_comb.u_b.out",       "aliases_sub_comb.u_a.client_out")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_comb.u_b.client_in", "aliases_sub_comb.u_a.in")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_comb.u_b.client_in", "aliases_sub_comb.u_a.client_out")));
}

// Same again, but this time with sub modules that register their inputs
// directly to outputs.
BOOST_AUTO_TEST_CASE(aliases_sub_reg) {
  BOOST_CHECK_NO_THROW(compile("aliases_sub_reg.sv"));
  netlist_paths::Options::getInstance().setRestrictStartPoints(false);
  netlist_paths::Options::getInstance().setRestrictEndPoints(false);
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_reg.u_a.out",       "aliases_sub_reg.u_b.in")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_reg.u_a.out",       "aliases_sub_reg.u_b.client_out")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_reg.u_b.out",       "aliases_sub_reg.u_a.in")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("aliases_sub_reg.u_b.out",       "aliases_sub_reg.u_a.client_out")));
}

//===----------------------------------------------------------------------===//
// Vlvbound bug
//===----------------------------------------------------------------------===//

/// Test that the inlined tasks do not share a merged VlVbound node.
/// See https://www.veripool.org/boards/3/topics/2619
BOOST_AUTO_TEST_CASE(vlvbound) {
  BOOST_CHECK_NO_THROW(compile("vlvbound.sv"));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("i_foo_current", "o_foo_inactive")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("i_foo_next", "o_next_foo_inactive")));
  BOOST_TEST(!np->pathExists(netlist_paths::Waypoints("i_foo_current", "o_next_foo_inactive")));
  BOOST_TEST(!np->pathExists(netlist_paths::Waypoints("i_foo_next", "o_foo_inactive")));
}

BOOST_AUTO_TEST_SUITE_END();