
#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test all paths query.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(paths_all_query, TestContext);

BOOST_AUTO_TEST_CASE(path_all_paths) {
  BOOST_CHECK_NO_THROW(compile("multiple_paths.sv"));
  auto paths = np->getAllPaths(netlist_paths::Waypoints("in", "out"));
  BOOST_TEST(paths.size() == 3);
  // Path 1
  BOOST_TEST(paths[0].length() == 5);
  CHECK_VAR_REPORT(paths[0].getVertex(0), "PORT", "logic", "in");
  CHECK_VAR_REPORT(paths[0].getVertex(2), "VAR",  "logic", "multiple_paths.a");
  CHECK_VAR_REPORT(paths[0].getVertex(4), "PORT", "logic", "out");
  // Path 2
  BOOST_TEST(paths[1].length() == 5);
  CHECK_VAR_REPORT(paths[1].getVertex(0), "PORT", "logic", "in");
  CHECK_VAR_REPORT(paths[1].getVertex(2), "VAR",  "logic", "multiple_paths.b");
  CHECK_VAR_REPORT(paths[1].getVertex(4), "PORT", "logic", "out");
  // Path 3
  BOOST_TEST(paths[2].length() == 5);
  CHECK_VAR_REPORT(paths[2].getVertex(0), "PORT", "logic", "in");
  CHECK_VAR_REPORT(paths[2].getVertex(2), "VAR",  "logic", "multiple_paths.c");
  CHECK_VAR_REPORT(paths[2].getVertex(4), "PORT", "logic", "out");
}

BOOST_AUTO_TEST_SUITE_END();
