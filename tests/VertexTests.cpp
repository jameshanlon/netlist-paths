#define BOOST_TEST_MODULE vertex_tests

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include "tests/definitions.hpp"
#include "TestContext.hpp"
#include "netlist_paths/Utilities.hpp"

//===----------------------------------------------------------------------===//
// Test querying of vertex types.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_CASE(registers, TestContext) {
  BOOST_CHECK_NO_THROW(compile("pipeline_module.sv"));
  BOOST_TEST(np->getRegVerticesPtr().size() == 8);
  BOOST_TEST(np->getPortVerticesPtr().size() == 8);
  BOOST_TEST(np->getNetVerticesPtr().size() == 50);
}

BOOST_FIXTURE_TEST_CASE(port_register, TestContext) {
  // A registered output is a port and register.
  BOOST_CHECK_NO_THROW(compile("registered_output.sv"));
  BOOST_TEST(np->getRegVerticesPtr().size() == 1);
  BOOST_TEST(np->getPortVerticesPtr().size() == 7);
  BOOST_TEST(np->getNetVerticesPtr().size() == 0);
  // Nested module with register type propagated to outer module.
  BOOST_CHECK_NO_THROW(compile("registered_output_nested.sv"));
  BOOST_TEST(np->getRegVerticesPtr().size() == 1);
  BOOST_TEST(np->getPortVerticesPtr().size() == 7);
  BOOST_TEST(np->getNetVerticesPtr().size() == 3);
}
