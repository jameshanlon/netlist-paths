#define BOOST_TEST_MODULE name_tests

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include "tests/definitions.hpp"
#include "TestContext.hpp"

/// Test matching of names by wildcards and regexes.
BOOST_FIXTURE_TEST_CASE(name_matching, TestContext) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv", "pipeline"));
  // Wildcard
  netlist_paths::Options::getInstance().setMatchWildcard();
  BOOST_TEST(np->regExists("*data_q*"));
  BOOST_TEST(np->regExists("*d?t?_q*"));
  BOOST_TEST(np->regExists("pipeline/*/data_q*"));
  BOOST_TEST(np->regExists("pipeline/*/*data_q*"));
  // Regex
  netlist_paths::Options::getInstance().setMatchRegex();
  BOOST_TEST(np->regExists(".*data_q.*"));
  BOOST_TEST(np->regExists(".*d.t._q.*"));
  BOOST_TEST(np->regExists("pipeline/.*/data_q.*"));
  BOOST_TEST(np->regExists("pipeline/.*/*data_q.*"));
  // Malformed regex
  BOOST_CHECK_THROW(np->regExists("*data_q"), netlist_paths::Exception);
  BOOST_CHECK_THROW(np->regExists("?data_q"), netlist_paths::Exception);
}
