import sys
import os
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
import py_netlist_paths

graph = py_netlist_paths.AnalyseGraph()
graph.parseFile(os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv'))
