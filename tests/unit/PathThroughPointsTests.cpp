#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test through points.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_through_points, TestContext);

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

BOOST_AUTO_TEST_SUITE_END();
