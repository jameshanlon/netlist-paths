import argparse
import sys
from py_netlist_paths import Netlist

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('netlist_file')
    args = parser.parse_args()
    netlist = Netlist(args.netlist_file)
    for register in netlist.get_reg_vertices():
        print('{} {}'.format(register.get_name(), register.get_dtype_str()))
