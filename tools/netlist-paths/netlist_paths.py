import argparse
import sys
import os
import tempfile
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
from py_netlist_paths import RunVerilator, Netlist, Options


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Query a Verilog netlist")
    parser.add_argument('files',
                        nargs='+',
                        help='Input files')
    parser.add_argument('-c', '--compile',
                        action='store_true',
                        help='Run Verilator to compile a netlist')
    parser.add_argument('-I',
                        metavar='include_path',
                        help='Add an source include path (only with --compile)')
    parser.add_argument('-D',
                        metavar='definition',
                        help='Define a preprocessor macro (only with --compile)')
    parser.add_argument('--dump-names',
                        nargs='?',
                        default='',
                        const='.*',
                        help='Dump all names, filter by regex')
    parser.add_argument('--from',
                        dest='start_point',
                        help='Start point')
    parser.add_argument('--to',
                        dest='end_point',
                        help='End point')
    parser.add_argument('--regex',
                        action='store_true',
                        help='Enable regular expression matching of names')
    parser.add_argument('-v', '--verbose',
                        action='store_true',
                        help='Print information')
    parser.add_argument('-d', '--debug',
                        action='store_true',
                        help='Print debugging information')
    args = parser.parse_args()
    # Setup options
    if args.regex:
        Options.get_instance().set_match_regex()
    if args.verbose:
        Options.get_instance().set_verbose()
    if args.debug:
        Options.get_instance().set_debug()
    # Verilator compilation
    if (args.compile):
        comp = RunVerilator(defs.INSTALL_PREFIX)
        temp_name = next(tempfile._get_candidate_names())
        comp.run(args.files[0], temp_name)
    # Create the netlist
    netlist = Netlist(temp_name)
    # Dump names
    if len(args.dump_names):
        netlist.dump_names(args.dump_names)
    # Point-to-point path
    if args.start_point and args.end_point:
        path = netlist.get_any_path(args.start_point, args.end_point)
        for vertex in path:
            print('{:<16} {:<16} {:<16} {:<16}'.format(vertex.get_ast_type(),
                                                       vertex.get_dtype_str(),
                                                       vertex.get_dtype_width(),
                                                       vertex.get_name()))
