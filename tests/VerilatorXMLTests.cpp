#define BOOST_TEST_MODULE verilator_xml_tests

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include "tests/definitions.hpp"
#include "TestContext.hpp"
#include "netlist_paths/Utilities.hpp"

/// Verilator cannot flatten modules with interfaces.
BOOST_FIXTURE_TEST_CASE(module_interface, TestContext) {
  BOOST_CHECK_THROW(compile("module_interface_inline.sv"), netlist_paths::XMLException);
}

/// Verilator cannot flatten public modules.
BOOST_FIXTURE_TEST_CASE(module_public_inline, TestContext) {
  BOOST_CHECK_THROW(compile("module_public_inline.sv"), netlist_paths::XMLException);
}

/// Verilator cannot flatten public modules.
BOOST_FIXTURE_TEST_CASE(module_no_inline, TestContext) {
  BOOST_CHECK_THROW(compile("module_no_inline.sv"), netlist_paths::XMLException);
}

/// Verilator cannot flatten modules with classes.
BOOST_FIXTURE_TEST_CASE(module_class_inline, TestContext) {
  BOOST_CHECK_THROW(compile("module_class_inline.sv"), netlist_paths::XMLException);
}

