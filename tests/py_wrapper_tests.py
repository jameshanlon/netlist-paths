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

    def test_counter(self):
        comp = py_netlist_paths.CompileGraph(defs.INSTALL_PREFIX)
        comp.run(os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv'), 'netlist.graph')
        graph = py_netlist_paths.Netlist()
        graph.parse_file('netlist.graph')
        self.assertTrue(graph.reg_exists('counter_q'))
        self.assertFalse(graph.reg_exists('foo'))

if __name__ == '__main__':
    unittest.main()
