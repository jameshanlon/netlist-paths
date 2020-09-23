import argparse
import sys
import os
sys.path.insert(0, os.path.join('${CMAKE_BINARY_DIR}', 'lib', 'netlist_paths'))
from py_netlist_paths import RunVerilator, Netlist, Options

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Query path in a Verilog netlist")
    args = parser.parse_args()
