import os
import sys
import unittest
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
from py_netlist_paths import RunVerilator, Netlist, Waypoints, Options

class TestPyWrapper(unittest.TestCase):
    """
    Test the Python API works as expected.
    """

    def setUp(self):
        pass

    def compile_test(self, files, includes=[], defines=[], top_module=""):
        """
        Compile a test and setup/reset options.
        """
        comp = RunVerilator(defs.INSTALL_PREFIX)
        if type(files) is list:
            _files = list(map(lambda p: os.path.join(defs.TEST_SRC_PREFIX, p), files))
        else:
            _files = [os.path.join(defs.TEST_SRC_PREFIX, files)]
        comp.run(includes, defines, _files, top_module, 'netlist.xml')
        Options.get_instance().set_error_on_unmatched_node(True)
        Options.get_instance().set_ignore_hierarchy_markers(False)
        Options.get_instance().set_match_exact()
        return Netlist('netlist.xml')

    def test_verilator_bin(self):
        self.assertTrue(os.path.exists(defs.INSTALL_PREFIX))

    def test_option_ignore_hierarchy_markers(self):
        np = self.compile_test('counter.sv')
        Options.get_instance().set_ignore_hierarchy_markers(True)
        self.assertTrue(np.path_exists(Waypoints('counter/counter_q', 'o_count')))
        self.assertTrue(np.path_exists(Waypoints('counter_counter_q', 'o_count')))

    def test_adder_path_points(self):
        """
        Test basic querying of path start and end points.
        """
        np = self.compile_test('adder.sv')
        # Start and finish points
        self.assertTrue(np.startpoint_exists('i_a'))
        self.assertTrue(np.startpoint_exists('i_b'))
        self.assertTrue(np.endpoint_exists('o_sum'))
        self.assertTrue(np.endpoint_exists('o_co'))
        # Any start and finish points.
        self.assertTrue(np.any_startpoint_exists('i_a'))
        self.assertTrue(np.any_startpoint_exists('i_b'))
        self.assertTrue(np.any_endpoint_exists('o_sum'))
        self.assertTrue(np.any_endpoint_exists('o_co'))
        Options.get_instance().set_match_regex()
        self.assertTrue(np.any_startpoint_exists('i_.*'))
        self.assertTrue(np.any_endpoint_exists('o_.*'))
        Options.get_instance().set_match_exact()
        # Invalid start and end points
        self.assertFalse(np.endpoint_exists('i_a'))
        self.assertFalse(np.endpoint_exists('i_b'))
        self.assertFalse(np.startpoint_exists('o_sum'))
        self.assertFalse(np.startpoint_exists('o_co'))

    def test_adder_path_exists(self):
        """
        Test basic querying of path existence.
        """
        np = self.compile_test('adder.sv')
        Options.get_instance().set_match_exact()
        # Check all valid paths are reported.
        self.assertTrue(np.path_exists(Waypoints('i_a', 'o_sum')))
        self.assertTrue(np.path_exists(Waypoints('i_a', 'o_co')))
        self.assertTrue(np.path_exists(Waypoints('i_b', 'o_sum')))
        self.assertTrue(np.path_exists(Waypoints('i_b', 'o_co')))
        # Check for invalid paths.
        self.assertRaises(RuntimeError, np.path_exists, Waypoints('o_sum', 'i_a'))
        self.assertRaises(RuntimeError, np.path_exists, Waypoints('o_co',  'i_a'))
        self.assertRaises(RuntimeError, np.path_exists, Waypoints('o_sum', 'i_b'))
        self.assertRaises(RuntimeError, np.path_exists, Waypoints('o_co',  'i_b'))

    def test_get_names(self):
        """
        Check get X vertices methods, with and without default arguments.
        """
        np = self.compile_test('counter.sv')
        Options.get_instance().set_match_regex()
        # Named vertices
        self.assertTrue(len(np.get_named_vertices()) == 12)
        self.assertTrue(len(np.get_named_vertices('counter_q')) == 1)
        self.assertTrue(len(np.get_named_vertices('foo')) == 0)
        # Register vertices
        self.assertTrue(len(np.get_reg_vertices()) == 1)
        self.assertTrue(len(np.get_reg_vertices('counter_q')) == 1)
        self.assertTrue(len(np.get_reg_vertices('foo')) == 0)
        # Net vertices
        self.assertTrue(len(np.get_net_vertices()) == 6)
        self.assertTrue(len(np.get_net_vertices('\.counter')) == 1)
        self.assertTrue(len(np.get_net_vertices('foo')) == 0)
        # Port vertices
        self.assertTrue(len(np.get_port_vertices()) == 4)
        self.assertTrue(len(np.get_port_vertices('clk')) == 1)
        self.assertTrue(len(np.get_port_vertices('foo')) == 0)

    def test_dtypes(self):
        """
        Check dtype queries (see C++ unit tests for complete set).
        """
        np = self.compile_test('dtypes.sv')
        self.assertTrue(np.get_dtype_width('logic') == 1)
        self.assertTrue(np.get_vertex_dtype_width('dtypes.logic_bit') == 1)
        self.assertTrue(np.get_vertex_dtype_str('dtypes.logic_bit') == 'logic')
        self.assertTrue(np.get_dtype_width('packed_struct_nested3_t') == 3+4+3)
        self.assertTrue(np.get_vertex_dtype_width('dtypes.packstruct_nested3') == 3+4+3)
        self.assertTrue(np.get_vertex_dtype_str('dtypes.packstruct_nested3') == 'packed struct')
        # Check that exceptions are raised
        self.assertRaises(RuntimeError, np.get_dtype_width, 'foo')
        self.assertRaises(RuntimeError, np.get_vertex_dtype_str, 'foo')
        self.assertRaises(RuntimeError, np.get_vertex_dtype_width, 'foo')

    def test_counter_regs(self):
        """
        Test querying of register existence.
        """
        np = self.compile_test('counter.sv')
        # Valid
        self.assertTrue(np.reg_exists('counter.counter_q'))
        self.assertTrue(np.any_reg_exists('counter.counter_q'))
        # Invalid
        self.assertFalse(np.reg_exists('counter.foo'))
        self.assertFalse(np.any_reg_exists('counter.foo'))

    def test_pipeline_module_regs(self):
        """
        Test querying of register existence.
        """
        np = self.compile_test('pipeline_module.sv')
        # Register exists for all pipeline stages.
        for i in list(range(8)):
            self.assertTrue(np.reg_exists('pipeline_module.g_pipestage['+str(i)+'].u_pipestage.data_q'))
        # Any register exists with regex pattern.
        Options.get_instance().set_match_regex()
        self.assertTrue(np.any_reg_exists('pipeline_module.g_pipestage\[.\].u_pipestage.data_q'))
        self.assertTrue(np.any_reg_exists('pipeline_module..*.u_pipestage.data_q'))

    def test_path_any_to_any(self):
        """
        Test querying of any paths.
        """
        # Pipeline
        np = self.compile_test('pipeline_loops.sv')
        path = np.get_any_path(Waypoints('i_data', 'pipeline_loops.data_q'))
        self.assertTrue(path.length() == 3)
        # Pipeline module
        np = self.compile_test('pipeline_module.sv')
        path = np.get_any_path(Waypoints('i_data', 'pipeline_module.g_pipestage[0].u_pipestage.data_q'))
        self.assertTrue(path.length() == 6)

    def test_path_all_any_to_any(self):
        """
        Test querying of all paths.
        """
        np = self.compile_test('multiple_paths.sv')
        paths = np.get_all_paths(Waypoints('in', 'out'))
        self.assertTrue(len(paths) == 3)

    def test_path_all_fanout(self):
        """
        Test querying of all fanout paths.
        """
        np = self.compile_test('fan_out_in.sv')
        paths = np.get_all_fanout_paths('in')
        self.assertTrue(len(paths) == 3)

    def test_path_all_fanin(self):
        """
        Test querying of all fanin paths.
        """
        np = self.compile_test('fan_out_in.sv')
        paths = np.get_all_fanin_paths('out')
        self.assertTrue(len(paths) == 3)

    def test_any_start_finish_points(self):
        """
        Test matching of distinct paths through common mid points.
        """
        np = self.compile_test('multiple_separate_paths.sv')
        Options.get_instance().set_match_wildcard()
        Options.get_instance().set_match_any_vertex()
        path = np.get_any_path(Waypoints('i_*', 'o_*'))
        self.assertTrue(not path.empty())
        path = np.get_any_path(Waypoints('i_b', 'o_b'))
        self.assertTrue(not path.empty())
        Options.get_instance().set_match_one_vertex()
        path = np.get_any_path(Waypoints('i_c', 'o_c'))
        self.assertTrue(not path.empty())

    def test_through_registers(self):
        """
        Test enabling path traversal through registers.
        """
        np = self.compile_test('basic_ff_chain.sv')
        Options.get_instance().set_traverse_registers(True)
        self.assertTrue(np.path_exists(Waypoints('in', 'out')))

    def test_restrict_start_end_points(self):
        """
        Test relaxation of path start/end point options.
        """
        np = self.compile_test('aliases_sub_reg.sv')
        Options.get_instance().set_restrict_start_points(False)
        Options.get_instance().set_restrict_end_points(False)
        self.assertTrue(np.path_exists(Waypoints('aliases_sub_reg.u_a.out', 'aliases_sub_reg.u_b.in')))
        self.assertTrue(np.path_exists(Waypoints('aliases_sub_reg.u_a.out', 'aliases_sub_reg.u_b.client_out')))

    def test_single_include(self):
        """
        Test passing an include path to Verilator
        """
        np = self.compile_test('single_include.sv', includes = [os.path.join(defs.TEST_SRC_PREFIX, "include_a")])
        path = np.get_any_path(Waypoints('data_i', 'data_o'))
        self.assertTrue(not path.empty())

    def test_multiple_includes(self):
        """
        Test passing multiple include paths to Verilator
        """
        np = self.compile_test('multiple_includes.sv', includes = [os.path.join(defs.TEST_SRC_PREFIX, "include_a"), os.path.join(defs.TEST_SRC_PREFIX, "include_b")])
        path = np.get_any_path(Waypoints('data_i', 'data_o'))
        self.assertTrue(not path.empty())

    def test_single_define(self):
        """
        Test passing a define to Verilator
        """
        np = self.compile_test('single_define.sv', defines = ['EXPR_A=data_i'])
        path = np.get_any_path(Waypoints('data_i', 'data_o'))
        self.assertTrue(not path.empty())

    def test_multiple_defines(self):
        """
        Test passing multiple defines, with and withput assignment to Verilator
        """
        np = self.compile_test('multiple_defines.sv', defines = ['MY_DEFINE', 'EXPR_A=data_i', 'EXPR_B=data_o', ])
        path = np.get_any_path(Waypoints('data_i', 'data_o'))
        self.assertTrue(not path.empty())

    def test_multiple_files(self):
        """
        Test passing multiple files to compile with Verilator
        """
        np = self.compile_test(['multiple_files.sv', 'include_a/include_a.sv', 'include_b/include_b.sv'])
        path = np.get_any_path(Waypoints('data_i', 'data_o'))
        self.assertTrue(not path.empty())

        np = self.compile_test(['include_a/include_a.sv', 'include_b/include_b.sv', 'multiple_files.sv'])
        path = np.get_any_path(Waypoints('data_i', 'data_o'))
        self.assertTrue(not path.empty())

    def test_multiple_tops(self):
        """
        Test '--top-module' by passing multiple independent modules to Verilator
        """

        files = ['top_a.sv', 'top_b.sv']
        np = self.compile_test(files, top_module='top_a')
        path = np.get_any_path(Waypoints('data_a_i', 'data_a_o'))
        self.assertTrue(not path.empty())

        np = self.compile_test(files, top_module='top_b')
        path = np.get_any_path(Waypoints('data_b_i', 'data_b_o'))
        self.assertTrue(not path.empty())

if __name__ == '__main__':
    unittest.main()
