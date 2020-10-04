import argparse
import sys
import os
import tempfile
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
from py_netlist_paths import RunVerilator, Netlist, Waypoints, Options


# Dump a table of names and their attributes matching regex to fd.
def dump_names(netlist, regex, fd):
    rows = []
    HDR = ['Name', 'Type', 'DType', 'Direction', 'Location']
    vertices = netlist.get_named_vertices(regex)
    vertices = sorted(vertices, key=lambda x: (x.get_name(),
                                               x.get_ast_type(),
                                               x.get_dtype_str(),
                                               x.get_direction()))
    for vertex in vertices:
        rows.append((vertex.get_name(),
                     vertex.get_ast_type(),
                     vertex.get_dtype_str(),
                     vertex.get_direction(),
                     vertex.get_location()))
    # Calculate max widths for each column
    widths = [len(x) for x in HDR]
    for row in rows:
        for i, col in enumerate(row):
            widths[i] = max(widths[i], len(col))
    fmt = ' '.join('{row['+str(i)+']:<{widths['+str(i)+']}}' for i in range(len(HDR)))
    fmt += '\n'
    fd.write(fmt.format(row=HDR, widths=widths))
    for row in rows:
        fd.write(fmt.format(row=row, widths=widths))

# Report the details of a path.
def dump_path_report(netlist, path, fd):
    for vertex in path:
        print('{:<16} {:<16} {:<16} {:<16}'.format(vertex.get_ast_type(),
                                                   vertex.get_dtype_str(),
                                                   vertex.get_dtype_width(),
                                                   vertex.get_name()))

# Report a list of paths
def dump_path_list_report(netlist, paths, fd):
    for i, path in enumerate(paths):
        print('Path {}'.format(i))
        dump_path_report(netlist, path, fd)

def main():
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
                        default=None,
                        const='.*',
                        help='Dump all names, filter by regex')
    parser.add_argument('--dump-dot',
                        action='store_true',
                        help='Dump a dotfile of the netlist\'s graph')
    parser.add_argument('--from',
                        dest='start_point',
                        help='Start point')
    parser.add_argument('--to',
                        dest='finish_point',
                        help='Finish point')
    parser.add_argument('--though',
                        nargs=2,
                        action='append',
                        dest='through_points',
                        default=[],
                        help='Though point')
    parser.add_argument('--all-paths',
                        action='store_true',
                        help='Find all paths between two points (exponential time)')
    parser.add_argument('--regex',
                        action='store_true',
                        help='Enable regular expression matching of names')
    parser.add_argument('-v', '--verbose',
                        action='store_true',
                        help='Print execution information')
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
    try:
        # Verilator compilation
        # (Only support one source file.)
        if (args.compile):
            comp = RunVerilator(defs.INSTALL_PREFIX)
            temp_name = next(tempfile._get_candidate_names())
            if comp.run(args.files[0], temp_name) > 0:
                raise RuntimeError('error compiling design')
        # Create the netlist
        netlist = Netlist(temp_name)
        # Remove the temporary XML file
        if (args.compile):
            os.remove(temp_name)
        # Dump names
        if args.dump_names:
            dump_names(netlist, args.dump_names, sys.stdout)
            return 0
        if args.dump_dot:
            netlist.dump_dot_file('graph.dot')
            return 0
        # Point-to-point path
        if args.start_point and args.finish_point:
            waypoints = Waypoints()
            waypoints.add_start_point(args.start_point)
            waypoints.add_finish_point(args.finish_point)
            [waypoints.add_through_point(point) for point in args.through_points]
            if args.all_paths:
                path = netlist.get_all_paths(waypoints)
                dump_path_list_report(netlist, path, sys.stdout)
            else:
                path = netlist.get_any_path(waypoints)
                dump_path_report(netlist, path, sys.stdout)
            return 0
        # Fan out paths
        if args.start_point and not args.finish_point:
            if len(args.through_points) > 0:
                raise RuntimeError('cannot specify through points with fanout paths')
            paths = netlist.get_all_fanout_paths(args.start_point)
            dump_path_list_report(netlist, paths, sys.stdout)
            return 0
        # Fan in paths
        if args.finish_point and not args.start_point:
            if len(args.through_points) > 0:
                raise RuntimeError('cannot specify through points with fanin paths')
            paths = netlist.get_all_fanin_paths(args.finish_point)
            dump_path_list_report(netlist, paths, sys.stdout)
            return 0
    except RuntimeError as e:
        print('Error: '+str(e))
        return 1


if __name__ == '__main__':
    sys.exit(main())
