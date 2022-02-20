#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"
#include "netlist_paths/Utilities.hpp"

BOOST_FIXTURE_TEST_SUITE(names, TestContext);

//===----------------------------------------------------------------------===//
// Test exact matching.
//===----------------------------------------------------------------------===//

BOOST_AUTO_TEST_CASE(exact_name_matching_counter) {
  BOOST_CHECK_NO_THROW(compile("counter.sv"));
  netlist_paths::Options::getInstance().setMatchExact();
  // Check the SRC and DST variants are not reported twice.
  BOOST_TEST(np->regExists("counter.counter_q"));
  // Register can be start or end point.
  BOOST_TEST(np->startpointExists("counter.counter_q"));
  BOOST_TEST(np->endpointExists("counter.counter_q"));
  // Output ports can only be endpoints.
  BOOST_TEST(!np->regExists("counter.o_count"));
  BOOST_TEST(np->endpointExists("o_count"));
  // Port enpoints are restricted to top instances, as above.
  BOOST_TEST(!np->endpointExists("counter.o_count"));
  // A name that doesn't exist.
  BOOST_TEST(!np->regExists("foo"));
  BOOST_TEST(!np->startpointExists("foo"));
  BOOST_TEST(!np->endpointExists("foo"));
}

BOOST_AUTO_TEST_CASE(exact_name_matching_pipeline_module) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv"));
  // Exact
  netlist_paths::Options::getInstance().setMatchExact();
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[0].u_pipestage.data_q"));
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[1].u_pipestage.data_q"));
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[2].u_pipestage.data_q"));
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[3].u_pipestage.data_q"));
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[4].u_pipestage.data_q"));
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[5].u_pipestage.data_q"));
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[6].u_pipestage.data_q"));
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[7].u_pipestage.data_q"));
}

//===----------------------------------------------------------------------===//
// Test wildcard matching.
//===----------------------------------------------------------------------===//

/// Test the implemenation of wildcard matching.
BOOST_AUTO_TEST_CASE(wildcard_matching) {
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "foo"));
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "fo?"));
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "?o?"));
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "???"));
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "f*"));
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "*o"));
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "*"));
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "**"));
  BOOST_TEST(netlist_paths::wildcardMatch("foo", "***"));
  BOOST_TEST(netlist_paths::wildcardMatch("dadadadado", "*do"));
  BOOST_TEST(netlist_paths::wildcardMatch("mississippi", "*sip*"));
  BOOST_TEST(netlist_paths::wildcardMatch("mississippi", "*s?p*"));
  BOOST_TEST(netlist_paths::wildcardMatch("mississippi", "*s*p*"));
  BOOST_TEST(netlist_paths::wildcardMatch("mississippi", "**s*p**"));
  BOOST_TEST(netlist_paths::wildcardMatch("mississippi", "mi*i*i*i"));
}

/// Test matching of names by wildcards and regexes.
BOOST_AUTO_TEST_CASE(wildcard_name_matching) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv"));
  // Wildcard
  netlist_paths::Options::getInstance().setMatchWildcard();
  BOOST_TEST(np->anyRegExists("*data_q*"));
  BOOST_TEST(np->anyRegExists("*d?t?_q*"));
  BOOST_TEST(np->anyRegExists("pipeline*data_q*"));

  // Check mutliple matching registers raises exception.
  BOOST_CHECK_THROW(np->regExists("pipeline_module.g_pipestage[?].u_pipestage.data_q"), netlist_paths::Exception);
}

//===----------------------------------------------------------------------===//
// Test regex matching.
//===----------------------------------------------------------------------===//

/// Test matching of names by wildcards and regexes.
BOOST_AUTO_TEST_CASE(regex_name_matching) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv"));
  // Regex
  netlist_paths::Options::getInstance().setMatchRegex();
  BOOST_TEST(np->anyRegExists(".*data_q.*"));
  BOOST_TEST(np->anyRegExists(".*d.t._q.*"));
  BOOST_TEST(np->anyRegExists("pipeline..*.data_q.*"));
  BOOST_TEST(np->anyRegExists("pipeline..*.*data_q.*"));

  // Check mutliple matching registers raises exception.
  BOOST_CHECK_THROW(np->regExists("pipeline_module.g_pipestage\\[.\\].u_pipestage.data_q"), netlist_paths::Exception);
}

/// Test malform regexes are caught.
BOOST_AUTO_TEST_CASE(malformed_regex) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv", "pipeline"));
  netlist_paths::Options::getInstance().setMatchRegex();
  BOOST_CHECK_THROW(np->anyRegExists("*data_q"), netlist_paths::Exception);
  BOOST_CHECK_THROW(np->anyRegExists("?data_q"), netlist_paths::Exception);
}

//===----------------------------------------------------------------------===//
// Test ignoring of heirarchy separators.
//===----------------------------------------------------------------------===//

/// counter
BOOST_AUTO_TEST_CASE(hierarchy_separators_counter) {
  BOOST_CHECK_NO_THROW(compile("counter.sv", "counter"));
  netlist_paths::Options::getInstance().setIgnoreHierarchyMarkers(true);

  netlist_paths::Options::getInstance().setMatchExact();
  BOOST_TEST(np->regExists("counter.counter_q")); // Heir dot
  BOOST_TEST(np->regExists("counter/counter_q")); // Heir slash
  BOOST_TEST(np->regExists("counter_counter_q")); // Flat

  netlist_paths::Options::getInstance().setMatchRegex();
  BOOST_TEST(np->regExists("counter.counter_q")); // Heir dot
  BOOST_TEST(np->regExists("counter/counter_q")); // Heir slash
  BOOST_TEST(np->regExists("counter_counter_q")); // Flat

  netlist_paths::Options::getInstance().setMatchWildcard();
  BOOST_TEST(np->regExists("counter.counter_q")); // Heir dot
  BOOST_TEST(np->regExists("counter/counter_q")); // Heir slash
  BOOST_TEST(np->regExists("counter_counter_q")); // Flat
}

/// pipeline_module
BOOST_AUTO_TEST_CASE(hierarchy_separators_pipeline_module) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv", "pipeline_module"));
  netlist_paths::Options::getInstance().setIgnoreHierarchyMarkers(true);

  netlist_paths::Options::getInstance().setMatchRegex();
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage\\[0\\].u_pipestage.data_q")); // Hier dot
  BOOST_TEST(np->regExists("pipeline_module/g_pipestage\\[0\\]/u_pipestage/data_q")); // Hier slash
  BOOST_TEST(np->regExists("pipeline_module_g_pipestage\\[0\\]_u_pipestage_data_q")); // Flat
  BOOST_TEST(np->regExists("pipeline_module/g_pipestage\\[0\\]_u_pipestage_data_q")); // Mixed

  netlist_paths::Options::getInstance().setMatchWildcard();
  BOOST_TEST(np->regExists("pipeline_module.g_pipestage[0].u_pipestage.data_q")); // Hier dot
  BOOST_TEST(np->regExists("pipeline_module/g_pipestage[0]/u_pipestage/data_q")); // Hier slash
  BOOST_TEST(np->regExists("pipeline_module_g_pipestage[0]_u_pipestage_data_q")); // Flat
  BOOST_TEST(np->regExists("pipeline_module/g_pipestage[0]_u_pipestage_data_q")); // Mixed

  netlist_paths::Options::getInstance().setMatchRegex();
  // With pipeline_module/.* prefix
  BOOST_TEST(np->anyRegExists("pipeline_module..*.u_pipestage.data_q")); // Hier dot
  BOOST_TEST(np->anyRegExists("pipeline_module/.*/u_pipestage/data_q")); // Hier slash
  BOOST_TEST(np->anyRegExists("pipeline_module_.*_u_pipestage_data_q")); // Flat
  // With .*/.* prefix
  BOOST_TEST(np->anyRegExists(".*..*.u_pipestage.data_q")); // Hier dot
  BOOST_TEST(np->anyRegExists(".*/.*/u_pipestage/data_q")); // Hier slash
  BOOST_TEST(np->anyRegExists(".*_.*_u_pipestage_data_q")); // Flat

  netlist_paths::Options::getInstance().setMatchWildcard();
  // With pipeline_module/* prefix
  BOOST_TEST(np->anyRegExists("pipeline_module.*.u_pipestage.data_q")); // Hier dot
  BOOST_TEST(np->anyRegExists("pipeline_module/*/u_pipestage/data_q")); // Hier slash
  BOOST_TEST(np->anyRegExists("pipeline_module_*_u_pipestage_data_q")); // Flat
  // With */* prefix
  BOOST_TEST(np->anyRegExists("*.*.u_pipestage.data_q")); // Hier dot
  BOOST_TEST(np->anyRegExists("*/*/u_pipestage/data_q")); // Hier slash
  BOOST_TEST(np->anyRegExists("*_*_u_pipestage_data_q")); // Flat
}

BOOST_AUTO_TEST_SUITE_END();
