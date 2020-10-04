import os
import sys
import unittest
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
from py_netlist_paths import RunVerilator, Netlist, Waypoints, Options

class TestPyWrapper(unittest.TestCase):

    def setUp(self):
        pass

    def compile_test(self, filename):
        comp = RunVerilator(defs.INSTALL_PREFIX)
        comp.run(os.path.join(defs.TEST_SRC_PREFIX, filename), 'netlist.xml')
        return Netlist('netlist.xml')

    def test_verilator_bin(self):
        self.assertTrue(os.path.exists(defs.INSTALL_PREFIX))

    def test_adder_regs(self):
        np = self.compile_test('adder.sv')
        for prefix in ['', 'adder.']:
            # Start and finish points
            self.assertTrue(np.startpoint_exists(prefix+'i_a'))
            self.assertTrue(np.startpoint_exists(prefix+'i_b'))
            self.assertTrue(np.endpoint_exists(prefix+'o_sum'))
            self.assertTrue(np.endpoint_exists(prefix+'o_co'))
            # Invalid start and end points
            self.assertFalse(np.endpoint_exists(prefix+'i_a'))
            self.assertFalse(np.endpoint_exists(prefix+'i_b'))
            self.assertFalse(np.startpoint_exists(prefix+'o_sum'))
            self.assertFalse(np.startpoint_exists(prefix+'o_co'))

    def test_adder_paths(self):
        np = self.compile_test('adder.sv')
        for prefix in ['', 'adder.']:
            # Check all valid paths are reported.
            self.assertTrue(np.path_exists(Waypoints(prefix+'i_a', prefix+'o_sum')))
            self.assertTrue(np.path_exists(Waypoints(prefix+'i_a', prefix+'o_co')))
            self.assertTrue(np.path_exists(Waypoints(prefix+'i_b', prefix+'o_sum')))
            self.assertTrue(np.path_exists(Waypoints(prefix+'i_b', prefix+'o_co')))
            # Check for invalid paths.
            self.assertRaises(RuntimeError, np.path_exists, Waypoints(prefix+'o_sum', prefix+'i_a'))
            self.assertRaises(RuntimeError, np.path_exists, Waypoints(prefix+'o_co',  prefix+'i_a'))
            self.assertRaises(RuntimeError, np.path_exists, Waypoints(prefix+'o_sum', prefix+'i_b'))
            self.assertRaises(RuntimeError, np.path_exists, Waypoints(prefix+'o_co',  prefix+'i_b'))

    def test_counter_regs(self):
        np = self.compile_test('counter.sv')
        # Variations on hierarchical paths
        self.assertTrue(np.reg_exists('counter_q'))
        self.assertTrue(np.reg_exists('counter.counter_q'))
        self.assertTrue(np.reg_exists('counter/counter_q'))
        self.assertTrue(np.reg_exists('counter_counter_q'))
        # Register can be start or end point.
        self.assertTrue(np.startpoint_exists('counter_q'))
        self.assertTrue(np.endpoint_exists('counter_q'))
        # Output ports can only be endpoints.
        self.assertFalse(np.reg_exists('counter.o_count'))
        self.assertTrue(np.endpoint_exists('counter.o_count'))
        self.assertTrue(np.endpoint_exists('o_count'))
        # A name that doesn't exist.
        self.assertFalse(np.reg_exists('foo'))
        self.assertFalse(np.startpoint_exists('foo'))
        self.assertFalse(np.endpoint_exists('foo'))

    def test_counter_paths(self):
        np = self.compile_test('counter.sv')
        # Check all valid paths are reported.
        self.assertTrue(np.path_exists(Waypoints('counter.i_clk', 'counter.counter_q')))
        self.assertTrue(np.path_exists(Waypoints('counter.i_rst', 'counter.counter_q')))
        self.assertTrue(np.path_exists(Waypoints('counter.counter_q', 'counter.o_count')))
        self.assertTrue(np.path_exists(Waypoints('counter.counter_q', 'counter.o_wrap')))
        # Check invalid paths.
        self.assertRaises(RuntimeError, np.path_exists, Waypoints('counter.o_count', 'counter.counter_q'))
        self.assertRaises(RuntimeError, np.path_exists, Waypoints('counter.count_q', 'counter.i_clk'))
        self.assertRaises(RuntimeError, np.path_exists, Waypoints('counter.count_q', 'counter.i_rst'))
        self.assertFalse(np.path_exists(Waypoints('counter.i_clk', 'counter.o_count')))
        self.assertFalse(np.path_exists(Waypoints('counter.i_clk', 'counter.o_wrap')))
        self.assertFalse(np.path_exists(Waypoints('counter.i_rst', 'counter.o_count')))
        self.assertFalse(np.path_exists(Waypoints('counter.i_rst', 'counter.o_wrap')))
        # TODO: check --from o_counter has no fan out paths

    def test_pipeline_module_regs(self):
        np = self.compile_test('pipeline_module.sv')
        # Register path variations
        self.assertTrue(np.reg_exists("pipeline_module.g_pipestage\\[0\\].u_pipestage.data_q")) # Hier dot
        self.assertTrue(np.reg_exists("pipeline_module/g_pipestage\\[0\\]/u_pipestage/data_q")) # Hier slash
        self.assertTrue(np.reg_exists("pipeline_module_g_pipestage\\[0\\]_u_pipestage_data_q")) # Flat
        self.assertTrue(np.reg_exists("pipeline_module/g_pipestage\\[0\\]_u_pipestage_data_q")) # Mixed
        self.assertTrue(np.reg_exists("g_pipestage\\[0\\]/u_pipestage_data_q")) # Mixed
        # Regexes
        self.assertTrue(np.reg_exists("pipeline_module/.*/u_pipestage_data_q"))
        self.assertTrue(np.reg_exists("pipeline_module/.*/data_q"))

    def test_pipeline_module_paths(self):
        np = self.compile_test('pipeline_module.sv')
        self.assertTrue(np.path_exists(Waypoints("pipeline_module.g_pipestage\\[0\\].u_pipestage.data_q", "pipeline_module.g_pipestage\\[1\\].u_pipestage.data_q")))
        self.assertTrue(np.path_exists(Waypoints("pipeline_module.g_pipestage\\[1\\].u_pipestage.data_q", "pipeline_module.g_pipestage\\[2\\].u_pipestage.data_q")))
        self.assertTrue(np.path_exists(Waypoints("pipeline_module.g_pipestage\\[2\\].u_pipestage.data_q", "pipeline_module.g_pipestage\\[3\\].u_pipestage.data_q")))
        self.assertTrue(np.path_exists(Waypoints("pipeline_module.g_pipestage\\[3\\].u_pipestage.data_q", "pipeline_module.g_pipestage\\[4\\].u_pipestage.data_q")))
        self.assertTrue(np.path_exists(Waypoints("pipeline_module.g_pipestage\\[4\\].u_pipestage.data_q", "pipeline_module.g_pipestage\\[5\\].u_pipestage.data_q")))
        self.assertTrue(np.path_exists(Waypoints("pipeline_module.g_pipestage\\[5\\].u_pipestage.data_q", "pipeline_module.g_pipestage\\[6\\].u_pipestage.data_q")))
        self.assertTrue(np.path_exists(Waypoints("pipeline_module.g_pipestage\\[6\\].u_pipestage.data_q", "pipeline_module.g_pipestage\\[7\\].u_pipestage.data_q")))

    def test_pipeline_paths(self):
        # A pipeline implementation that uses loops and a register array.
        # Registers within the array cannot be distinguished.
        np = self.compile_test('pipeline_loops.sv')
        self.assertTrue(np.reg_exists('pipeline_loops.data_q'))
        self.assertTrue(np.path_exists(Waypoints('pipeline_loops.i_data', 'pipeline_loops.data_q')))
        self.assertTrue(np.path_exists(Waypoints('pipeline_loops.data_q', 'pipeline_loops.o_data')))

    def test_vlvbound(self):
        np = self.compile_test('vlvbound.sv')
        # Test that the inlined tasks do not share a merged VlVbound node.
        # See https://www.veripool.org/boards/3/topics/2619
        self.assertTrue(np.path_exists(Waypoints("i_foo_current", "o_foo_inactive")))
        self.assertTrue(np.path_exists(Waypoints("i_foo_next", "o_next_foo_inactive")))
        self.assertFalse(np.path_exists(Waypoints("i_foo_current", "o_next_foo_inactive")))
        self.assertFalse(np.path_exists(Waypoints("i_foo_next", "o_foo_inactive")))

    def test_name_matching_option(self):
        np = self.compile_test("adder.sv")
        Options.get_instance().set_match_wildcard()
        # ...
        Options.get_instance().set_match_regex()
        # ...

    def test_dtypes(self):
        # Check dtype queries (see C++ unit tests for complete set).
        np = self.compile_test('dtypes.sv')
        self.assertTrue(np.get_dtype_width('logic') == 1)
        self.assertTrue(np.get_vertex_dtype_width('logic_bit') == 1)
        self.assertTrue(np.get_vertex_dtype_str('logic_bit') == 'logic')
        self.assertTrue(np.get_dtype_width('packed_struct_nested3_t') == 3+4+3)
        self.assertTrue(np.get_vertex_dtype_width('packstruct_nested3') == 3+4+3)
        self.assertTrue(np.get_vertex_dtype_str('packstruct_nested3') == 'packed struct')
        # Check that exceptions are raised
        self.assertRaises(RuntimeError, np.get_dtype_width, 'foo')
        self.assertRaises(RuntimeError, np.get_vertex_dtype_str, 'foo')
        self.assertRaises(RuntimeError, np.get_vertex_dtype_width, 'foo')

    def test_path_any_to_any(self):
        # Pipeline
        np = self.compile_test('pipeline_loops.sv')
        path = np.get_any_path(Waypoints('i_data', 'data_q'))
        self.assertTrue(len(path) == 3)
        # TODO: check the paths
        # Pipeline module
        np = self.compile_test('pipeline_module.sv')
        path = np.get_any_path(Waypoints('i_data', 'data_q'))
        self.assertTrue(len(path) == 7)
        # TODO: check the paths

    def test_path_all_any_to_any(self):
        np = self.compile_test('multiple_paths.sv')
        paths = np.get_all_paths(Waypoints('in', 'out'))
        self.assertTrue(len(paths) == 3)
        # TODO: check the paths

    def test_path_all_fanout(self):
        np = self.compile_test('fan_out_in.sv')
        paths = np.get_all_fanout_paths('in')
        self.assertTrue(len(paths) == 3)
        # TODO: check the paths

    def test_path_all_fanin(self):
        np = self.compile_test('fan_out_in.sv')
        paths = np.get_all_fanin_paths('out')
        self.assertTrue(len(paths) == 3)
        # TODO: check the paths


if __name__ == '__main__':
    unittest.main()
