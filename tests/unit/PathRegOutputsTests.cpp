#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test matching of paths through modules with registered outputs (a registered
// output is a port and register). Because of the way that Verilator inlines the
// modules, the source names of the resisters are not preserved. However, there
// should be the correct number of registers and paths.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_reg_outputs, TestContext);

BOOST_AUTO_TEST_CASE(paths_with_port_registers) {

  BOOST_CHECK_NO_THROW(compile("registered_output_path.sv"));

  // There are three registers.
  BOOST_TEST(np->getRegVerticesPtr().size() == 3);

  // Check that REG_ALIASES match against 'is register' queries.
  BOOST_TEST(np->regExists("registered_output_path.u_foo1.o_b"));
  BOOST_TEST(np->regExists("registered_output_path.u_foo2.o_b"));
  //BOOST_TEST(np->regExists("registered_output_path.u_foo3.o_b")); // Should be a REG_ALIAS
  BOOST_TEST(np->anyRegExists("registered_output_path.u_foo1.o_b"));
  BOOST_TEST(np->anyRegExists("registered_output_path.u_foo2.o_b"));
  //BOOST_TEST(np->anyRegExists("registered_output_path.u_foo3.o_b")); // Should be a REG_ALIAS

  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("in",
                                                     "registered_output_path.u_foo1.o_b")));
  BOOST_TEST(np->pathExists(netlist_paths::Waypoints("registered_output_path.u_foo1.o_b",
                                                     "registered_output_path.u_foo2.o_b")));
  // FIXME: u_foo3.o_b not marked as register, needs investigation.
  //BOOST_TEST(np->pathExists(netlist_paths::Waypoints("registered_output_path.u_foo2.o_b",
  //                                                   "registered_output_path.u_foo3.o_b")));
  //BOOST_TEST(np->pathExists(netlist_paths::Waypoints("registered_output_path.u_foo3.o_b",
  //                                                   "out")));

  // Fan out from source registers to a destination register plus two aliases.
  BOOST_TEST(np->getAllFanOut("in").size() == 3);
  BOOST_TEST(np->getAllFanOut("registered_output_path.data1").size() == 3);
  BOOST_TEST(np->getAllFanOut("registered_output_path.data2").size() == 1);
}

BOOST_AUTO_TEST_SUITE_END();
