
#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test path exists queries
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_exists_query, TestContext);

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

BOOST_AUTO_TEST_SUITE_END();
