
#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"

//===----------------------------------------------------------------------===//
// Test path queries through registers.
//===----------------------------------------------------------------------===//

BOOST_FIXTURE_TEST_SUITE(path_through_regs_queries, TestContext);

/// Basic tests for through-registers traveral of basic_ff_chain.sv
BOOST_AUTO_TEST_CASE(through_registers) {
  BOOST_CHECK_NO_THROW(compile("basic_ff_chain.sv"));

  // Paths through regisers.
  {
    // Any path: in -> a -> b -> out
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto path = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
    BOOST_TEST(path.length() == 7);
    CHECK_VAR_REPORT(path.getVertex(0), "PORT", "logic", "in");
    CHECK_VAR_REPORT(path.getVertex(2), "DST_REG", "logic", "basic_ff_chain.a");
    CHECK_VAR_REPORT(path.getVertex(4), "DST_REG", "logic", "basic_ff_chain.b");
    CHECK_VAR_REPORT(path.getVertex(6), "PORT", "logic", "out");
  }
  {
    // Any path: a -> b -> out
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.a", "out"));
    BOOST_TEST(path.length() == 5);
    CHECK_VAR_REPORT(path.getVertex(0), "SRC_REG", "logic", "basic_ff_chain.a");
    CHECK_VAR_REPORT(path.getVertex(2), "DST_REG", "logic", "basic_ff_chain.b");
    CHECK_VAR_REPORT(path.getVertex(4), "PORT", "logic", "out");
  }
  {
    // Any path: b -> out
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.b", "out"));
    BOOST_TEST(path.length() == 3);
    CHECK_VAR_REPORT(path.getVertex(0), "SRC_REG", "logic", "basic_ff_chain.b");
    CHECK_VAR_REPORT(path.getVertex(2), "PORT", "logic", "out");
  }

  // Check also the through-register paths don't appear when not traversing registers.
  {
    netlist_paths::Options::getInstance().setTraverseRegisters(false);
    auto path = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
    BOOST_TEST(path.empty());
  }
  {
    netlist_paths::Options::getInstance().setTraverseRegisters(false);
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.a", "out"));
    BOOST_TEST(path.empty());
  }
  {
    netlist_paths::Options::getInstance().setTraverseRegisters(false);
    auto path = np->getAnyPath(netlist_paths::Waypoints("basic_ff_chain.a", "out"));
    BOOST_TEST(path.empty());
  }

  // Check fanout and fanin path queries.
  {
    // Fanout: in
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto paths = np->getAllFanOut("in");
    std::vector<std::string> endPoints = {"out", "basic_ff_chain.a", "basic_ff_chain.b"};
    BOOST_TEST(paths.size() == endPoints.size());
    for (auto path : paths) {
      BOOST_TEST((std::find(endPoints.begin(), endPoints.end(), path.getFinishVertex()->getName()) != endPoints.end()));
    }
  }
  {
    // Fanout: out
    netlist_paths::Options::getInstance().setTraverseRegisters(true);
    auto paths = np->getAllFanIn("out");
    std::vector<std::string> startPoints = {"in", "basic_ff_chain.a", "basic_ff_chain.b"};
    BOOST_TEST(paths.size() == startPoints.size());
    for (auto path : paths) {
      BOOST_TEST((std::find(startPoints.begin(), startPoints.end(), path.getStartVertex()->getName()) != startPoints.end()));
    }
  }
}

/// A similar test case but with the register start, through and finish points
/// all being port registers.
BOOST_AUTO_TEST_CASE(through_reg_ports) {
  BOOST_CHECK_NO_THROW(compile("registered_output_path.sv"));
  netlist_paths::Options::getInstance().setTraverseRegisters(true);
  auto path = np->getAnyPath(netlist_paths::Waypoints("in", "out"));
  BOOST_TEST(path.length() == 7);
  // This vertex is a PORT and a REG.
  CHECK_VAR_REPORT(path.getVertex(6), "DST_REG", "[31:0] logic", "out");
  BOOST_TEST(path.getVertex(6)->getSimpleAstTypeStr() == "PORT_REG");
}

BOOST_AUTO_TEST_SUITE_END();
