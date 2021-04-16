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

//===----------------------------------------------------------------------===//
// Exercise the special case of register ports (when a register is top and has
// a direction), since these interact badly with the way Verilator inlines
// modules.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_CASE(port_register, TestContext) {
  BOOST_CHECK_NO_THROW(compile("registered_output.sv"));
  netlist_paths::Options::getInstance().setMatchRegex();
  // Names
  BOOST_TEST(np->getNamedVerticesPtr().size() == 6);
  // Register (just one, no aliases)
  BOOST_TEST(np->getRegVerticesPtr().size() == 1);
  BOOST_TEST(np->regExists("o_b"));
  BOOST_TEST(np->anyRegExists("o_b"));
  // Ports
  BOOST_TEST(np->getPortVerticesPtr().size() == 6);
  BOOST_TEST(np->getPortVerticesPtr("i_").size() == 4);
  BOOST_TEST(np->getPortVerticesPtr("o_").size() == 2);
  // Nets
  BOOST_TEST(np->getNetVerticesPtr().size() == 0);
}

BOOST_FIXTURE_TEST_CASE(port_register_nested, TestContext) {

  // Nested module with register type propagated to outer module.
  BOOST_CHECK_NO_THROW(compile("registered_output_nested.sv"));
  netlist_paths::Options::getInstance().setMatchRegex();
  // Names
  BOOST_TEST(np->getNamedVerticesPtr().size() == 9);
  // Regs. Note that inlining of foo causes registered_output_nested.u_foo.o_b
  // to be merged into o_b.
  BOOST_TEST(np->getRegVerticesPtr().size() == 1);
  BOOST_TEST(np->regExists("o_b"));
  BOOST_TEST(np->anyRegExists("o_b"));
  // Ports
  BOOST_TEST(np->getPortVerticesPtr().size() == 6);
  BOOST_TEST(np->getPortVerticesPtr("i_").size() == 4);
  BOOST_TEST(np->getPortVerticesPtr("o_").size() == 2);
  // Nets
  BOOST_TEST(np->getNetVerticesPtr().size() == 3);
  BOOST_TEST(np->getNetVerticesPtr("i_").size() == 2);
  BOOST_TEST(np->getNetVerticesPtr("o_").size() == 1);
}
