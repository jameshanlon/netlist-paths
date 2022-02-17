#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test handling of aliases and path start/end point restrictions.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_start_finish_aliases, TestContext);

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

BOOST_AUTO_TEST_SUITE_END();
