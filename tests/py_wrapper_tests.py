import sys
import os
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
import py_netlist_paths

comp = py_netlist_paths.CompileGraph(defs.INSTALL_PREFIX)
print (os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv'))
comp.run(os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv'), 'netlist.graph')

graph = py_netlist_paths.AnalyseGraph()
graph.parseFile('netlist.graph')
