#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test matching any start/finish points.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_match_any_query, TestContext);

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

BOOST_AUTO_TEST_SUITE_END();
