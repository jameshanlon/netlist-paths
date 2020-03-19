import os
import sys
import unittest
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
import py_netlist_paths

class TeastPyWrapper(unittest.TestCase):

    def setUp(self):
        pass

    def test_verilator_bin(self):
        self.assertTrue(os.path.exists(defs.INSTALL_PREFIX))

    def test_adder(self):
        comp = py_netlist_paths.CompileGraph(defs.INSTALL_PREFIX)
        comp.run(os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv'), 'netlist.graph')
        graph = py_netlist_paths.Netlist()
        graph.parse_file('netlist.graph')
        # Check all valid paths are reported.
        self.assertTrue(graph.path_exists('adder.i_a', 'adder.o_sum'))
        self.assertTrue(graph.path_exists('adder.i_a', 'adder.o_co'))
        self.assertTrue(graph.path_exists('adder.i_b', 'adder.o_sum'))
        self.assertTrue(graph.path_exists('adder.i_b', 'adder.o_co'))
        # Check for invalid paths.
        self.assertFalse(graph.path_exists('adder.o_sum', 'adder.i_a'))
        self.assertFalse(graph.path_exists('adder.o_co',  'adder.i_a'))
        self.assertFalse(graph.path_exists('adder.o_sum', 'adder.i_b'))
        self.assertFalse(graph.path_exists('adder.o_co',  'adder.i_b'))

    def test_counter(self):
        comp = py_netlist_paths.CompileGraph(defs.INSTALL_PREFIX)
        comp.run(os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv'), 'netlist.graph')
        graph = py_netlist_paths.Netlist()
        graph.parse_file('netlist.graph')
        # Register can be start or end point.
        self.assertTrue(graph.reg_exists('counter_q'))
        self.assertTrue(graph.startpoint_exists('counter_q'))
        self.assertTrue(graph.endpoint_exists('counter_q'))
        # Output port can be endpoint only.
        self.assertFalse(graph.reg_exists('counter.o_count'))
        self.assertTrue(graph.endpoint_exists('counter.o_count'))
        # A name that doesn't exist.
        self.assertFalse(graph.reg_exists('foo'))
        self.assertFalse(graph.startpoint_exists('foo'))
        self.assertFalse(graph.endpoint_exists('foo'))
        # Check all valid paths are reported.
        self.assertTrue(graph.path_exists('counter.i_clk',     'counter.counter_q'));
        self.assertTrue(graph.path_exists('counter.i_rst',     'counter.counter_q'));
        self.assertTrue(graph.path_exists('counter.counter_q', 'counter.o_count'));
        # Check invalid paths.
        self.assertFalse(graph.path_exists('counter.o_count', 'counter.counter_q'));
        self.assertFalse(graph.path_exists('counter.count_q', 'counter.i_clk'));
        self.assertFalse(graph.path_exists('counter.count_q', 'counter.i_rst'));

if __name__ == '__main__':
    unittest.main()
