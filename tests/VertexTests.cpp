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
  netlist_paths::Options::getInstance().setMatchRegex();
  // Names
  BOOST_TEST(np->getNamedVerticesPtr().size() == 60);
  // Regs
  BOOST_TEST(np->getRegVerticesPtr().size() == 8);
  BOOST_TEST(np->getRegVerticesPtr("data_q").size() == 8);
  // Ports
  BOOST_TEST(np->getPortVerticesPtr().size() == 8);
  BOOST_TEST(np->getPortVerticesPtr("i_").size() == 6);
  BOOST_TEST(np->getPortVerticesPtr("o_").size() == 2);
  // Nets
  BOOST_TEST(np->getNetVerticesPtr().size() == 50);
  BOOST_TEST(np->getNetVerticesPtr("clk").size() == 8);
  BOOST_TEST(np->getNetVerticesPtr("rst").size() == 8);
  BOOST_TEST(np->getNetVerticesPtr("data").size() == 32);
}

BOOST_FIXTURE_TEST_CASE(port_register, TestContext) {

  // A registered output is a port and register.
  BOOST_CHECK_NO_THROW(compile("registered_output.sv"));
  netlist_paths::Options::getInstance().setMatchRegex();
  // Names
  BOOST_TEST(np->getNamedVerticesPtr().size() == 6);
  // Regs
  BOOST_TEST(np->getRegVerticesPtr().size() == 1);
  // Ports
  BOOST_TEST(np->getPortVerticesPtr().size() == 7);
  BOOST_TEST(np->getPortVerticesPtr("i_").size() == 4);
  BOOST_TEST(np->getPortVerticesPtr("o_").size() == 3);
  // Nets
  BOOST_TEST(np->getNetVerticesPtr().size() == 0);

  // Nested module with register type propagated to outer module.
  BOOST_CHECK_NO_THROW(compile("registered_output_nested.sv"));
  netlist_paths::Options::getInstance().setMatchRegex();
  // Names
  BOOST_TEST(np->getNamedVerticesPtr().size() == 9);
  // Regs
  BOOST_TEST(np->getRegVerticesPtr().size() == 1);
  // Ports
  BOOST_TEST(np->getPortVerticesPtr().size() == 7);
  BOOST_TEST(np->getPortVerticesPtr("i_").size() == 4);
  BOOST_TEST(np->getPortVerticesPtr("o_").size() == 3);
  // Nets
  BOOST_TEST(np->getNetVerticesPtr().size() == 3);
  BOOST_TEST(np->getNetVerticesPtr("i_").size() == 2);
  BOOST_TEST(np->getNetVerticesPtr("o_").size() == 1);
}
