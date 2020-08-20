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
  // Logic declarations with packed and unpacked arrays.
  BOOST_TEST(np->getDTypeStr("dtypes.logic_bit") == "logic");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_vector") == "[3:0] logic");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_packarray_vector") == "[3:0] [2:0] logic");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_packarray2d_vector") == "[3:0] [2:0] [1:0] logic");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray") == "logic [3:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray2d") == "logic [3:0] [2:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray3d") == "logic [3:0] [2:0] [1:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray_vector") == "[3:0] logic [2:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_unpackarray2d_vector") == "[4:0] [3:0] logic [2:0] [1:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.logic_array_large") == "[212934:123421] logic [213412:112312]");
  // Struct declarations with packed and unpacked arrays.
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_notypedef") == "packed struct");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct") == "packed struct");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_packarray") == "[7:0] packed struct");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_packarray2d") == "[7:0] [6:0] packed struct");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_unpackarray") == "packed struct [7:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_unpackarray2d") == "packed struct [7:0] [6:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_packarray_unpackarray") == "[7:0] packed struct [6:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_packarray2d_unpackarray2d") == "[7:0] [6:0] packed struct [5:0] [4:0]");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_nested") == "packed struct");
  BOOST_TEST(np->getDTypeStr("dtypes.packstruct_nested2") == "packed struct");
  // Enums
  // Note that the <enum> node is not referenced by the dtype in the XML, so the
  // dtype is reported as a logic vector.
  BOOST_TEST(np->getDTypeStr("dtypes.enum_auto") == "[2:0] logic");
  BOOST_TEST(np->getDTypeStr("dtypes.enum_onehot") == "[2:0] logic");
  BOOST_TEST(np->getDTypeStr("dtypes.enum_onehot_packarray2d_unpackarray2d") == "[5:0] [4:0] [2:0] logic [3:0] [1:0]");
  // Unions
  BOOST_TEST(np->getDTypeStr("dtypes.union_logic_notypedef") == "packed union");
  BOOST_TEST(np->getDTypeStr("dtypes.union_struct_notypedef") == "packed union");
  BOOST_TEST(np->getDTypeStr("dtypes.union_logic_packarray2d_unpackarray2d") == "[4:0] [3:0] packed union [2:0] [1:0]");
}
