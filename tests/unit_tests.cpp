#define BOOST_TEST_MODULE compile_graph

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <fstream>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "netlist_paths/Netlist.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/ReadVerilatorXML.hpp"
#include "netlist_paths/RunVerilator.hpp"
#include "tests/definitions.hpp"
#include "TestContext.hpp"

namespace fs = boost::filesystem;

netlist_paths::Options options;

BOOST_FIXTURE_TEST_CASE(verilator, TestContext) {
  // Check the Verilator binary exists.
  BOOST_ASSERT(boost::filesystem::exists(installPrefix));
}

BOOST_FIXTURE_TEST_CASE(adder, TestContext) {
  BOOST_CHECK_NO_THROW(compile("adder.sv", "adder"));
  // Check paths between all start and end points are reported.
  auto startPoints = {"i_a", "i_b"};
  auto endPoints = {"o_sum", "o_co"};
  for (auto s : startPoints) {
    for (auto e : endPoints) {
      BOOST_TEST(pathExists(s, e));
      BOOST_TEST(!pathExists(e, s));
    }
  }
}

/// Test string representations of types.
BOOST_FIXTURE_TEST_CASE(dtypes, TestContext) {
  BOOST_CHECK_NO_THROW(compile("dtypes.sv", "dtypes"));
  BOOST_TEST(np->getDTypeStr("dtypes.logic_bit") == "logic");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_vector") == "[3:0] logic");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_packarray_vector") == "[3:0] [2:0] logic");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_packarray2d_vector") == "[3:0] [2:0] [1:0] logic");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray") == "logic [3:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray2d") == "logic [3:0] [2:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray3d") == "logic [3:0] [2:0] [1:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray_vector") == "[3:0] logic [1:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_array_large") == "[2129:1234] [81:74] logic [2134:1123] [74:23]");
}
