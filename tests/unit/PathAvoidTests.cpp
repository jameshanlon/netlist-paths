#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test path avoid points.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_avoid_points, TestContext);

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
    waypoints.addThroughPoint("in"); // Not a valid avoid point since it's a start point.
    BOOST_CHECK_THROW(np->getAllPaths(waypoints), netlist_paths::Exception);
  }
  {
    auto waypoints = netlist_paths::Waypoints("in", "out");
    waypoints.addThroughPoint("out"); // Not a valid avoid point since it's a finish point.
    BOOST_CHECK_THROW(np->getAllPaths(waypoints), netlist_paths::Exception);
  }
}

BOOST_AUTO_TEST_SUITE_END();
