import os
import sys
import unittest
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
import py_netlist_paths

class TestPyWrapper(unittest.TestCase):

    def setUp(self):
        pass

    def compile_test(self, filename):
        comp = py_netlist_paths.RunVerilator(defs.INSTALL_PREFIX)
        comp.run(os.path.join(defs.TEST_SRC_PREFIX, filename), 'netlist.xml')
        return py_netlist_paths.NetlistPaths('netlist.xml')

    def test_verilator_bin(self):
        self.assertTrue(os.path.exists(defs.INSTALL_PREFIX))

    def test_adder_regs(self):
        np = self.compile_test('adder.sv')
        for prefix in ['', 'adder.']:
            # Start and end points
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
            self.assertTrue(np.path_exists(prefix+'i_a', prefix+'o_sum'))
            self.assertTrue(np.path_exists(prefix+'i_a', prefix+'o_co'))
            self.assertTrue(np.path_exists(prefix+'i_b', prefix+'o_sum'))
            self.assertTrue(np.path_exists(prefix+'i_b', prefix+'o_co'))
            # Check for invalid paths.
            self.assertFalse(np.path_exists(prefix+'o_sum', prefix+'i_a'))
            self.assertFalse(np.path_exists(prefix+'o_co',  prefix+'i_a'))
            self.assertFalse(np.path_exists(prefix+'o_sum', prefix+'i_b'))
            self.assertFalse(np.path_exists(prefix+'o_co',  prefix+'i_b'))

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
        self.assertTrue(np.path_exists('counter.i_clk', 'counter.counter_q'))
        self.assertTrue(np.path_exists('counter.i_rst', 'counter.counter_q'))
        self.assertTrue(np.path_exists('counter.counter_q', 'counter.o_count'))
        self.assertTrue(np.path_exists('counter.counter_q', 'counter.o_wrap'))
        # Check invalid paths.
        self.assertFalse(np.path_exists('counter.o_count', 'counter.counter_q'))
        self.assertFalse(np.path_exists('counter.count_q', 'counter.i_clk'))
        self.assertFalse(np.path_exists('counter.count_q', 'counter.i_rst'))
        self.assertFalse(np.path_exists('counter.i_clk', 'counter.o_count'))
        self.assertFalse(np.path_exists('counter.i_clk', 'counter.o_wrap'))
        self.assertFalse(np.path_exists('counter.i_rst', 'counter.o_count'))
        self.assertFalse(np.path_exists('counter.i_rst', 'counter.o_wrap'))
        # TODO: check --from o_counter has no fan out paths

    def test_pipeline_module_regs(self):
        np = self.compile_test('pipeline_module.sv')
        # Register path variations
        self.assertTrue(np.reg_exists("pipeline.g_pipestage\\[0\\].u_pipestage.data_q")) # Hier dot
        self.assertTrue(np.reg_exists("pipeline/g_pipestage\\[0\\]/u_pipestage/data_q")) # Hier slash
        self.assertTrue(np.reg_exists("pipeline_g_pipestage\\[0\\]_u_pipestage_data_q")) # Flat
        self.assertTrue(np.reg_exists("pipeline/g_pipestage\\[0\\]_u_pipestage_data_q")) # Mixed
        self.assertTrue(np.reg_exists("g_pipestage\\[0\\]/u_pipestage_data_q")) # Mixed
        # Regexes
        self.assertTrue(np.reg_exists("pipeline/.*/u_pipestage_data_q"))
        self.assertTrue(np.reg_exists("pipeline/.*/data_q"))

    def test_pipeline_module_paths(self):
        np = self.compile_test('pipeline_module.sv')
        self.assertTrue(np.path_exists("pipeline.g_pipestage\\[0\\].u_pipestage.data_q", "pipeline.g_pipestage\\[1\\].u_pipestage.data_q"))
        self.assertTrue(np.path_exists("pipeline.g_pipestage\\[1\\].u_pipestage.data_q", "pipeline.g_pipestage\\[2\\].u_pipestage.data_q"))
        self.assertTrue(np.path_exists("pipeline.g_pipestage\\[2\\].u_pipestage.data_q", "pipeline.g_pipestage\\[3\\].u_pipestage.data_q"))
        self.assertTrue(np.path_exists("pipeline.g_pipestage\\[3\\].u_pipestage.data_q", "pipeline.g_pipestage\\[4\\].u_pipestage.data_q"))
        self.assertTrue(np.path_exists("pipeline.g_pipestage\\[4\\].u_pipestage.data_q", "pipeline.g_pipestage\\[5\\].u_pipestage.data_q"))
        self.assertTrue(np.path_exists("pipeline.g_pipestage\\[5\\].u_pipestage.data_q", "pipeline.g_pipestage\\[6\\].u_pipestage.data_q"))
        self.assertTrue(np.path_exists("pipeline.g_pipestage\\[6\\].u_pipestage.data_q", "pipeline.g_pipestage\\[7\\].u_pipestage.data_q"))

    def test_pipeline_paths(self):
        # A pipeline implementation that uses loops and a register array.
        # Registers within the array cannot be distinguished.
        np = self.compile_test('pipeline.sv')
        self.assertTrue(np.reg_exists('pipeline.data_q'))
        self.assertTrue(np.path_exists('pipeline.i_data', 'pipeline.data_q'))
        self.assertTrue(np.path_exists('pipeline.data_q', 'pipeline.o_data'))

    def test_vlvbound(self):
        np = self.compile_test('vlvbound.sv')
        # Test that the inlined tasks do not share a merged VlVbound node.
        # See https://www.veripool.org/boards/3/topics/2619
        self.assertTrue(np.path_exists("i_foo_current", "o_foo_inactive"))
        self.assertTrue(np.path_exists("i_foo_next", "o_next_foo_inactive"))
        self.assertFalse(np.path_exists("i_foo_current", "o_next_foo_inactive"))
        self.assertFalse(np.path_exists("i_foo_next", "o_foo_inactive"))

    def test_basic_types(self):
        # Check handling of basic types.
        np = self.compile_test('basic_types.sv')

    def test_fsm(self):
        np = self.compile_test('fsm.sv')

    def test_mux2(self):
        np = self.compile_test('mux2.sv')


if __name__ == '__main__':
    unittest.main()
