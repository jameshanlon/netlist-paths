#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Vlvbound bug
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(vlvbound_bug, TestContext);

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
