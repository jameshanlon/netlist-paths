#include <boost/test/unit_test.hpp>
#include "tests/unit/definitions.hpp"
#include "TestContext.hpp"
#include "netlist_paths/Utilities.hpp"

BOOST_FIXTURE_TEST_SUITE(names, TestContext);

/// Verilator cannot inline packages with functions.
BOOST_AUTO_TEST_CASE(orphan_package) {
  BOOST_CHECK_NO_THROW(compile("orphan_package.sv"));
  BOOST_TEST(!np->isEmpty());
}

/// Verilator cannot inline classes.
BOOST_AUTO_TEST_CASE(module_class_no_inline) {
  BOOST_CHECK_NO_THROW(compile("module_class_no_inline.sv"));
  BOOST_TEST(!np->isEmpty());
}

/// Verilator cannot inline interfaces.
BOOST_AUTO_TEST_CASE(interface_no_inline) {
  BOOST_CHECK_NO_THROW(compile("interface_no_inline.sv"));
  BOOST_TEST(np->isEmpty());
}

/// Verilator can flatten public modules.
BOOST_AUTO_TEST_CASE(module_public_no_inline) {
  BOOST_CHECK_NO_THROW(compile("public_module_no_inline.sv"));
  BOOST_TEST(!np->isEmpty());
}

/// Verilator can flatten public modules.
BOOST_AUTO_TEST_CASE(module_no_inline_pragma) {
  BOOST_CHECK_NO_THROW(compile("module_no_inline_pragma.sv"));
  BOOST_TEST(!np->isEmpty());
}

/// Verilator can order variable declarations after their references in the
/// typetable.
BOOST_AUTO_TEST_CASE(dtype_forward_refs) {
  BOOST_CHECK_NO_THROW(load("dtype_forward_refs.xml"));
}

/// Verilator can introduce new variables that are the target of delayed
/// assignments. Since the LHS of delayed assignments are how registers are
/// identified, register types are propagated through assign aliases.
BOOST_AUTO_TEST_CASE(assign_alias_regs) {
  BOOST_CHECK_NO_THROW(load("assign_alias_regs.xml"));
  BOOST_TEST(np->regExists("assign_alias_regs.sum.add.register_q"));
  BOOST_TEST(np->regExists("assign_alias_regs.__Vcellout__sum.add__register_q"));
}

/// Verilator can introduce references to variables in packages that do not
/// have a corresponding variable declaration in a varscope. See
/// https://github.com/jameshanlon/netlist-paths/issues/7
BOOST_AUTO_TEST_CASE(var_no_scope) {
  netlist_paths::Options::getInstance().setLoggingErrorOnly(); // Supress warning about missing scope.
  BOOST_CHECK_NO_THROW(compile("var_no_scope.sv"));
  BOOST_TEST(!np->isEmpty());
  netlist_paths::Options::getInstance().setLoggingQuiet();
}

BOOST_AUTO_TEST_SUITE_END();
