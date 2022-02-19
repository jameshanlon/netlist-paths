#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test fan in/out
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_fan_in_out_query, TestContext);

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
  CHECK_VAR_REPORT(paths[0].getVertex(0), "PORT", "logic", "in");
  CHECK_VAR_REPORT(paths[0].getVertex(2), "DST_REG", "logic", "fan_out_in.a");
  // Path 2
  BOOST_TEST(paths[1].length() == 3);
  CHECK_VAR_REPORT(paths[1].getVertex(0), "PORT", "logic", "in");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "DST_REG", "logic", "fan_out_in.b");
  // Path 3
  BOOST_TEST(paths[2].length() == 3);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "PORT", "logic", "in");
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
  CHECK_VAR_REPORT(paths[0].getVertex(2), "PORT", "logic", "out");
  // Path 2
  BOOST_TEST(paths[1].length() == 3);
  CHECK_VAR_REPORT(paths[1].getVertex(0), "SRC_REG", "logic", "fan_out_in.b");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "PORT", "logic", "out");
  // Path 3
  BOOST_TEST(paths[2].length() == 3);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "SRC_REG", "logic", "fan_out_in.c");
  CHECK_VAR_REPORT(paths[2].getVertex(2), "PORT", "logic", "out");
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
  CHECK_VAR_REPORT(paths[0].getVertex(0), "PORT", "logic", "in");
  CHECK_VAR_REPORT(paths[0].getVertex(2), "DST_REG", "logic", "fan_out_in_modules.foo_a.x");
  // Path 2
  BOOST_TEST(paths[1].length() == 3);
  CHECK_VAR_REPORT(paths[1].getVertex(0), "PORT", "logic", "in");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "DST_REG", "logic", "fan_out_in_modules.foo_b.x");
  // Path 3
  BOOST_TEST(paths[2].length() == 3);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "PORT", "logic", "in");
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
  CHECK_VAR_REPORT(paths[0].getVertex(5), "PORT", "logic", "out");
  BOOST_TEST(paths[1].length() == 6);\
  CHECK_VAR_REPORT(paths[1].getVertex(0), "SRC_REG", "logic", "fan_out_in_modules.foo_b.x");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "VAR", "logic", "fan_out_in_modules.b");
  CHECK_VAR_REPORT(paths[1].getVertex(4), "VAR", "logic", "fan_out_in_modules.foo_b.out");
  CHECK_VAR_REPORT(paths[1].getVertex(5), "PORT", "logic", "out");
  BOOST_TEST(paths[2].length() == 6);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "SRC_REG", "logic", "fan_out_in_modules.foo_c.x");
  CHECK_VAR_REPORT(paths[2].getVertex(2), "VAR", "logic", "fan_out_in_modules.c");
  CHECK_VAR_REPORT(paths[2].getVertex(4), "VAR", "logic", "fan_out_in_modules.foo_c.out");
  CHECK_VAR_REPORT(paths[2].getVertex(5), "PORT", "logic", "out");
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

BOOST_AUTO_TEST_SUITE_END();
