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

    def compile_test(self, filename):
        comp = RunVerilator(defs.INSTALL_PREFIX)
        comp.run(os.path.join(defs.TEST_SRC_PREFIX, filename), 'netlist.xml')
        Options.get_instance().set_respect_hierarchy_markers()
        Options.get_instance().set_match_exact()
        return Netlist('netlist.xml')

    def test_verilator_bin(self):
        self.assertTrue(os.path.exists(defs.INSTALL_PREFIX))

    def test_adder_regs(self):
        np = self.compile_test('adder.sv')
        Options.get_instance().set_match_exact()
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
        Options.get_instance().set_match_exact()
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

    def test_dtypes(self):
        # Check dtype queries (see C++ unit tests for complete set).
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

    def test_path_any_to_any(self):
        # Pipeline
        np = self.compile_test('pipeline_loops.sv')
        path = np.get_any_path(Waypoints('i_data', 'pipeline_loops.data_q'))
        self.assertTrue(len(path) == 3)
        # Pipeline module
        np = self.compile_test('pipeline_module.sv')
        path = np.get_any_path(Waypoints('i_data', 'pipeline_module.g_pipestage[0].u_pipestage.data_q'))
        self.assertTrue(len(path) == 7)

    def test_path_all_any_to_any(self):
        np = self.compile_test('multiple_paths.sv')
        paths = np.get_all_paths(Waypoints('in', 'out'))
        self.assertTrue(len(paths) == 3)

    def test_path_all_fanout(self):
        np = self.compile_test('fan_out_in.sv')
        paths = np.get_all_fanout_paths('in')
        self.assertTrue(len(paths) == 3)

    def test_path_all_fanin(self):
        np = self.compile_test('fan_out_in.sv')
        paths = np.get_all_fanin_paths('out')
        self.assertTrue(len(paths) == 3)


if __name__ == '__main__':
    unittest.main()
