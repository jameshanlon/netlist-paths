#define BOOST_TEST_MODULE unit_tests

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <fstream>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "tests/definitions.hpp"
#include "TestContext.hpp"

namespace fs = boost::filesystem;

BOOST_FIXTURE_TEST_CASE(verilator, TestContext) {
  // Check the Verilator binary exists.
  BOOST_ASSERT(boost::filesystem::exists(installPrefix));
}

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

/// Test string representations of types.
BOOST_FIXTURE_TEST_CASE(dtype_strings, TestContext) {
  BOOST_CHECK_NO_THROW(compile("dtypes.sv", "dtypes"));
  // Logic declarations with packed and unpacked arrays.
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_bit") == "logic");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_vector") == "[3:0] logic");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_packarray_vector") == "[3:0] [2:0] logic");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_packarray2d_vector") == "[3:0] [2:0] [1:0] logic");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_unpackarray") == "logic [3:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_unpackarray2d") == "logic [3:0] [2:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_unpackarray3d") == "logic [3:0] [2:0] [1:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_unpackarray_vector") == "[3:0] logic [2:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_unpackarray2d_vector") == "[4:0] [3:0] logic [2:0] [1:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.logic_array_large") == "[212934:123421] logic [213412:112312]");
  // Struct declarations with packed and unpacked arrays.
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_notypedef") == "packed struct");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct") == "packed struct");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_packarray") == "[7:0] packed struct");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_packarray2d") == "[7:0] [6:0] packed struct");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_unpackarray") == "packed struct [7:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_unpackarray2d") == "packed struct [7:0] [6:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_packarray_unpackarray") == "[7:0] packed struct [6:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_packarray2d_unpackarray2d") == "[7:0] [6:0] packed struct [5:0] [4:0]");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_nested") == "packed struct");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_nested2") == "packed struct");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.packstruct_nested3") == "packed struct");
  // Enums
  // Note that the <enum> node is not referenced by the dtype in the XML, so the
  // dtype is reported as a logic vector.
  BOOST_TEST(np->getVertexDTypeStr("dtypes.enum_notypedef") == "logic");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.enum_auto") == "[1:0] logic");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.enum_onehot") == "[2:0] logic");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.enum_onehot_packarray2d_unpackarray2d") == "[5:0] [4:0] [2:0] logic [3:0] [1:0]");
  // Unions
  BOOST_TEST(np->getVertexDTypeStr("dtypes.union_logic_notypedef") == "packed union");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.union_struct_notypedef") == "packed union");
  BOOST_TEST(np->getVertexDTypeStr("dtypes.union_logic_packarray2d_unpackarray2d") == "[4:0] [3:0] packed union [2:0] [1:0]");
  // Exceptions
  BOOST_CHECK_THROW(np->getVertexDTypeStr("dtypes.foo"), netlist_paths::Exception);
}

BOOST_FIXTURE_TEST_CASE(dtype_widths, TestContext) {
  BOOST_CHECK_NO_THROW(compile("dtypes.sv", "dtypes"));
  // Logic
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_bit") == 1);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_vector") == 4);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_packarray_vector") == 4*3);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_packarray2d_vector") == 4*3*2);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_unpackarray") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_unpackarray2d") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_unpackarray3d") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_unpackarray_vector") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_unpackarray2d_vector") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.logic_array_large") == 0);
  // Structs
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_notypedef") == 2);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct") == 2);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_packarray") == 2*8);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_packarray2d") == 2*8*7);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_unpackarray") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_unpackarray2d") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_packarray_unpackarray") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_packarray2d_unpackarray2d") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_nested") == 3);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_nested2") == 3+1);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.packstruct_nested3") == 3+4+3);
  // Enums
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.enum_notypedef") == 1);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.enum_auto") == 2);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.enum_onehot") == 3);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.enum_onehot_packarray2d_unpackarray2d") == 0);
  // Unions
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.union_logic_notypedef") == 1);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.union_struct_notypedef") == 2);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.union_logic_packarray2d_unpackarray2d") == 0);
  BOOST_TEST(np->getVertexDTypeWidth("dtypes.union_enum_struct") == 3);
  // Access via dtype names
  BOOST_TEST(np->getDTypeWidth("logic") == 1);
  BOOST_TEST(np->getDTypeWidth("packed_struct_t") == 2);
  BOOST_TEST(np->getDTypeWidth("packed_struct_nested_t") == 3);
  BOOST_TEST(np->getDTypeWidth("packed_struct_nested2_t") == 3+1);
  BOOST_TEST(np->getDTypeWidth("packed_struct_nested3_t") == 3+4+3);
  BOOST_TEST(np->getDTypeWidth("enum_auto_t") == 2);
  BOOST_TEST(np->getDTypeWidth("enum_onehot_t") == 3);
  BOOST_TEST(np->getDTypeWidth("union_logic_t") == 1);
  // Exceptions
  BOOST_CHECK_THROW(np->getVertexDTypeWidth("dtypes.foo"), netlist_paths::Exception);
  BOOST_CHECK_THROW(np->getDTypeWidth("dtypes.foo"), netlist_paths::Exception);
}

BOOST_FIXTURE_TEST_CASE(path_exists, TestContext) {
  BOOST_CHECK_NO_THROW(compile("adder.sv", "adder"));
  // Check paths between all start and end points are reported.
  auto startPoints = {"i_a", "i_b"};
  auto endPoints = {"o_sum", "o_co"};
  for (auto s : startPoints) {
    for (auto e : endPoints) {
      BOOST_TEST(pathExists(s, e));
      BOOST_CHECK_THROW(pathExists(e, s), netlist_paths::Exception);
    }
  }
}

void checkVarReport(const netlist_paths::Vertex *vertex,
                    const std::string astTypeStr,
                    const std::string dtypeStr,
                    const std::string name) {
  BOOST_TEST(vertex->getAstTypeStr() == astTypeStr);
  BOOST_TEST(vertex->getDTypeStr() == dtypeStr);
  BOOST_TEST(vertex->getName() == name);
}

void checkLogReport(const netlist_paths::Vertex *vertex,
                    const std::string astTypeStr) {
  BOOST_TEST(vertex->getAstTypeStr() == astTypeStr);
}

BOOST_FIXTURE_TEST_CASE(path_query_basic_assign_chain, TestContext) {
  BOOST_CHECK_NO_THROW(compile("basic_assign_chain.sv", "basic_assign_chain"));
  auto vertices = np->getAnyPath("in", "out");
  BOOST_TEST(vertices.size() == 7);
  checkVarReport(vertices[0], "VAR", "logic", "in");
  checkLogReport(vertices[1], "ASSIGN");
  checkVarReport(vertices[2], "VAR", "logic", "basic_assign_chain.a");
  checkLogReport(vertices[3], "ASSIGN");
  checkVarReport(vertices[4], "VAR", "logic", "basic_assign_chain.b");
  checkLogReport(vertices[5], "ASSIGN");
  checkVarReport(vertices[6], "VAR", "logic", "out");
}

BOOST_FIXTURE_TEST_CASE(path_query_basic_comb_chain, TestContext) {
  BOOST_CHECK_NO_THROW(compile("basic_comb_chain.sv", "basic_comb_chain"));
  auto vertices = np->getAnyPath("in", "out");
  BOOST_TEST(vertices.size() == 7);
  checkVarReport(vertices[0], "VAR", "logic", "in");
  checkLogReport(vertices[1], "ASSIGN");
  checkVarReport(vertices[2], "VAR", "logic", "basic_comb_chain.a");
  checkLogReport(vertices[3], "ASSIGN");
  checkVarReport(vertices[4], "VAR", "logic", "basic_comb_chain.b");
  checkLogReport(vertices[5], "ASSIGN");
  checkVarReport(vertices[6], "VAR", "logic", "out");
}

BOOST_FIXTURE_TEST_CASE(path_query_basic_ff_chain, TestContext) {
  BOOST_CHECK_NO_THROW(compile("basic_ff_chain.sv", "basic_ff_chain"));
  // in -> a
  auto vertices = np->getAnyPath("in", "basic_ff_chain.a");
  BOOST_TEST(vertices.size() == 3);
  checkVarReport(vertices[0], "VAR", "logic", "in");
  checkLogReport(vertices[1], "ASSIGN_DLY");
  checkVarReport(vertices[2], "DST_REG", "logic", "basic_ff_chain.a");
  // a -> b
  vertices = np->getAnyPath("basic_ff_chain.a", "basic_ff_chain.b");
  BOOST_TEST(vertices.size() == 3);
  checkVarReport(vertices[0], "SRC_REG", "logic", "basic_ff_chain.a");
  checkLogReport(vertices[1], "ASSIGN_DLY");
  checkVarReport(vertices[2], "DST_REG", "logic", "basic_ff_chain.b");
  // b -> out
  vertices = np->getAnyPath("basic_ff_chain.b", "out");
  BOOST_TEST(vertices.size() == 3);
  checkVarReport(vertices[0], "SRC_REG", "logic", "basic_ff_chain.b");
  checkLogReport(vertices[1], "ASSIGN");
  checkVarReport(vertices[2], "VAR", "logic", "out");
}

BOOST_FIXTURE_TEST_CASE(path_query_pipeline_module, TestContext) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv", "pipeline"));
  // NOTE: can differentiate between the generate instances of the pipeline.
  auto vertices = np->getAnyPath("i_data", "data_q");
  BOOST_TEST(vertices.size() == 7);
  checkVarReport(vertices[0], "VAR", "[31:0] logic", "i_data");
  checkLogReport(vertices[1], "ASSIGN");
  checkVarReport(vertices[2], "VAR", "[31:0] logic [8:0]", "pipeline.routing");
  checkLogReport(vertices[3], "ASSIGN");
  checkVarReport(vertices[4], "VAR", "[31:0] logic", "pipeline.__Vcellinp__g_pipestage[0].u_pipestage__i_data");
  checkLogReport(vertices[5], "ASSIGN_DLY");
  checkVarReport(vertices[6], "DST_REG", "[31:0] logic", "pipeline.g_pipestage[0].u_pipestage.data_q");
}

BOOST_FIXTURE_TEST_CASE(path_query_pipeline_loops, TestContext) {
  BOOST_CHECK_NO_THROW(compile("pipeline_loops.sv", "pipeline_loops"));
  // NOTE: cannot currently differentiate between elements of the data_q array.
  auto vertices = np->getAnyPath("i_data", "data_q");
  BOOST_TEST(vertices.size() == 3);
  checkVarReport(vertices[0], "VAR", "[31:0] logic", "i_data");
  checkLogReport(vertices[1], "ASSIGN_DLY");
  checkVarReport(vertices[2], "DST_REG", "[31:0] logic [7:0]", "pipeline_loops.data_q");
}

BOOST_FIXTURE_TEST_CASE(path_query_pipeline_no_loops, TestContext) {
  BOOST_CHECK_NO_THROW(compile("pipeline_no_loops.sv", "pipeline_no_loops"));
  // NOTE: cannot currently differentiate between elements of the data_q array.
  auto vertices = np->getAnyPath("data_q", "data_q");
  BOOST_TEST(vertices.size() == 3);
  checkVarReport(vertices[0], "SRC_REG", "[31:0] logic [2:0]", "pipeline_no_loops.data_q");
  checkLogReport(vertices[1], "ASSIGN_DLY");
  checkVarReport(vertices[2], "DST_REG", "[31:0] logic [2:0]", "pipeline_no_loops.data_q");
}
