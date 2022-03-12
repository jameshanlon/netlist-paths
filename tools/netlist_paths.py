#!/usr/bin/env python3

import argparse
import sys
import os
from itertools import zip_longest
import tempfile
import definitions as defs
sys.path.insert(0, os.path.join(defs.BINARY_DIR_PREFIX, 'lib', 'netlist_paths'))
from py_netlist_paths import RunVerilator, Netlist, Waypoints, Options


DEFAULT_DOT_FILE = 'graph.dot'

def write_table(rows, fd):
    """
    Write the table rows out to fd and calculate max widths for each column.
    """
    widths = [0] * len(rows[0])
    for row in rows:
        for i, col in enumerate(row):
            widths[i] = max(widths[i], len(col))
    fmt = ' '.join('{row['+str(i)+']:<{widths['+str(i)+']}}' for i in range(len(rows[0])))
    fmt += '\n'
    fd.write(fmt.format(row=rows[0], widths=widths))
    fd.write(fmt.format(row=['-'*w for w in widths], widths=widths))
    for row in rows[1:]:
        fd.write(fmt.format(row=row, widths=widths))

def dump_names(vertices, fd):
    """
    Dump a table of vertex names and their attributes matching regex to fd.
    """
    rows = []
    HDR = ['Name', 'Type', 'DType', 'Width', 'Direction', 'Location']
    rows.append(HDR)
    vertices = sorted(vertices, key=lambda x: (x.get_name(),
                                               x.get_ast_type_str(),
                                               x.get_dtype_str(),
                                               x.get_dtype_width(),
                                               x.get_direction_str(),
                                               x.get_location_str()))
    for vertex in vertices:
        rows.append((vertex.get_name(),
                     vertex.get_ast_type_str(),
                     vertex.get_dtype_str(),
                     str(vertex.get_dtype_width()),
                     vertex.get_direction_str(),
                     vertex.get_location_str()))
    if len(vertices) > 0:
        # Write the table out.
        write_table(rows, fd)
    else:
        fd.write('No matching vertices.\n')

def dump_dtypes(dtypes, fd):
    """
    Dump a table of data types.
    """
    rows = []
    HDR = ['Name', 'Width', 'Description']
    rows.append(HDR)
    # Sort by name, then description.
    dtypes = sorted(dtypes, key=lambda x: (x.get_name(), x.to_str()))
    for dtype in dtypes:
      rows.append((dtype.get_name(),
                   str(dtype.get_width()),
                   dtype.to_str()))
    if len(dtypes) > 0:
        # Write the table out.
        write_table(rows, fd)
    else:
        fd.write('No matching data types.\n')

def dump_path_report(netlist, path, fd):
    """
    Report the details of a path. Items in the path alternate between variable
    and statement, starting and ending with a variable.
    """
    rows = []
    rows.append(('Name', 'Type', 'DType', 'Statement', 'Location'))
    index = 0
    while index < path.length():
        # Var reference and logic statement.
        vcurrent = path.get_vertex(index)
        vnext = path.get_vertex(index+1)
        if index+1 < path.length() and \
           not vcurrent.is_logic() and \
           vnext.is_logic():
            row = (vcurrent.get_name(),
                   vcurrent.get_ast_type_str(),
                   vcurrent.get_dtype_str(),
                   vnext.get_ast_type_str(),
                   vnext.get_location_str())
            index += 2
        # Var reference only.
        elif not vcurrent.is_logic():
            row = (vcurrent.get_name(),
                   vcurrent.get_ast_type_str(),
                   vcurrent.get_dtype_str(),
                   '',
                   '')
            index += 1
        # Statement only.
        else:
            row = ('', '', '', vcurrent.get_ast_type_str(), vcurrent.get_location_str())
            index += 1
        rows.append(row)
    if len(rows) > 1:
        # Write the table out.
        write_table(rows, fd)
    else:
        print('No matching paths.')

def dump_path_list_report(netlist, paths, fd):
    """
    Report a list of paths.
    """
    if len(paths) == 0:
        print('No matching paths.')
        return
    for i, path in enumerate(paths):
        fd.write('\nPath {}\n'.format(i))
        dump_path_report(netlist, path, fd)

def main():
    parser = argparse.ArgumentParser(description="Query a Verilog netlist")
    parser.add_argument('files',
                        nargs='+',
                        help='Input files')
    parser.add_argument('--version',
                        action='version',
                        version='%(prog)s {}.{}.{}'.format(defs.VERSION_MAJOR, defs.VERSION_MINOR, defs.VERSION_PATCH),
                        help='Display the version number and exit')
    parser.add_argument('-c', '--compile',
                        action='store_true',
                        help='Run Verilator to compile a netlist')
    parser.add_argument('-I',
                        default=[],
                        nargs='*',
                        metavar='include_path',
                        help='Add an source include path (only with --compile)')
    parser.add_argument('-D',
                        default=[],
                        nargs='*',
                        metavar='definition',
                        help='Define a preprocessor macro (only with --compile)')
    parser.add_argument('-t', '--top-module',
                        default="",
                        dest='top_module',
                        metavar='top module',
                        help='Specify top module for compilation')
    parser.add_argument('-o', '--output',
                        default=None,
                        dest='output_file',
                        metavar='output file',
                        help='Specify an output file')
    parser.add_argument('--dump-names',
                        nargs='?',
                        default=None,
                        const='',
                        metavar='pattern',
                        help='Dump all named entities, filter by regex')
    parser.add_argument('--dump-nets',
                        nargs='?',
                        default=None,
                        const='',
                        metavar='pattern',
                        help='Dump all nets, filter by regex')
    parser.add_argument('--dump-ports',
                        nargs='?',
                        default=None,
                        const='',
                        metavar='pattern',
                        help='Dump all ports, filter by regex')
    parser.add_argument('--dump-regs',
                        nargs='?',
                        default=None,
                        const='',
                        metavar='pattern',
                        help='Dump all registers, filter by regex')
    parser.add_argument('--dump-dtypes',
                        nargs='?',
                        default=None,
                        const='',
                        metavar='pattern',
                        help='Dump all data types, filter by regex')
    parser.add_argument('--dump-dot',
                        action='store_true',
                        help='Dump a dotfile of the netlist\'s graph')
    parser.add_argument('--from',
                        dest='start_point',
                        metavar='point',
                        help='Specify a path start point')
    parser.add_argument('--to',
                        dest='finish_point',
                        metavar='point',
                        help='Specify a path finish point')
    parser.add_argument('--through',
                        action='append',
                        default=[],
                        dest='through_points',
                        metavar='point',
                        help='Specify a path though point')
    parser.add_argument('--avoid',
                        action='append',
                        default=[],
                        dest='avoid_points',
                        metavar='point',
                        help='Specify a point for a path to avoid')
    parser.add_argument('--traverse-registers',
                        action='store_const',
                        const=lambda: Options.get_instance().set_traverse_registers(True),
                        default=lambda *args: None,
                        help='Allow paths to traverse registers')
    parser.add_argument('--start-anywhere',
                        action='store_const',
                        const=lambda: Options.get_instance().set_restrict_start_points(False),
                        default=lambda *args: None,
                        help='Allow paths to start on any variable')
    parser.add_argument('--end-anywhere',
                        action='store_const',
                        const=lambda: Options.get_instance().set_restrict_end_points(False),
                        default=lambda *args: None,
                        help='Allow paths to end on any variable')
    parser.add_argument('--all-paths',
                        action='store_true',
                        help='Find all paths between two points (exponential time)')
    parser.add_argument('--regex',
                        action='store_const',
                        const=lambda: Options.get_instance().set_match_regex(),
                        default=lambda *args: None,
                        help='Enable regular expression matching of names')
    parser.add_argument('--wildcard',
                        action='store_const',
                        const=lambda: Options.get_instance().set_match_wildcard(),
                        default=lambda *args: None,
                        help='Enable wildcard matching of names')
    parser.add_argument('--ignore-hierarchy-markers',
                        action='store_const',
                        const=lambda: Options.get_instance().set_ignore_hierarchy_markers(True),
                        default=lambda *args: None,
                        help='Ignore hierarchy markers: _ . /')
    parser.add_argument('-v', '--verbose',
                        action='store_const',
                        const=lambda: Options.get_instance().set_logging_verbose(),
                        default=lambda *args: None,
                        help='Print execution information')
    parser.add_argument('-d', '--debug',
                        action='store_const',
                        const=lambda: Options.get_instance().set_logging_debug(),
                        default=lambda *args: None,
                        help='Print debugging information')
    args = parser.parse_args()

    # Setup options.
    args.traverse_registers()
    args.regex()
    args.wildcard()
    args.ignore_hierarchy_markers()
    args.start_anywhere()
    args.end_anywhere()
    args.verbose()
    args.debug()

    try:
        # Verilator compilation
        # (Only supports one source file currently, useful for testing.)
        if args.compile:
            if args.output_file == None:
                output_filename = next(tempfile._get_candidate_names())
            else:
                output_filename = args.output_file
            comp = RunVerilator(defs.INSTALL_PREFIX)
            if comp.run(args.I, args.D, args.files, args.top_module, output_filename) > 0:
                raise RuntimeError('error compiling design')
        else:
            if len(args.files) != 1:
                raise RuntimeError('cannot specify multiple netlist XML files')
            output_filename = args.files[0]

        # Create the netlist
        netlist = Netlist(output_filename)

        # Delete the temporary XML output file.
        if args.compile and args.output_file == None:
            os.remove(output_filename)

        # Dump all names
        if args.dump_names != None:
            dump_names(netlist.get_named_vertices(args.dump_names), sys.stdout)
            return 0

        # Dump nets
        if args.dump_nets != None:
            dump_names(netlist.get_net_vertices(args.dump_nets), sys.stdout)
            return 0

        # Dump ports
        if args.dump_ports != None:
            dump_names(netlist.get_port_vertices(args.dump_ports), sys.stdout)
            return 0

        # Dump regs
        if args.dump_regs != None:
            dump_names(netlist.get_reg_vertices(args.dump_regs), sys.stdout)
            return 0

        # Dump data types
        if args.dump_dtypes != None:
            dump_dtypes(netlist.get_named_dtypes(args.dump_dtypes), sys.stdout)
            return 0

        # Dump graph dotfile
        if args.dump_dot:
            netlist.dump_dot_file(args.output_file if args.output_file else DEFAULT_DOT_FILE)
            return 0

        # Point-to-point path
        if args.start_point and args.finish_point:
            waypoints = Waypoints()
            waypoints.add_start_point(args.start_point)
            waypoints.add_finish_point(args.finish_point)
            [waypoints.add_through_point(point) for point in args.through_points]
            [waypoints.add_avoid_point(point) for point in args.avoid_points]
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
            if len(args.avoid_points) > 0:
                raise RuntimeError('cannot specify avoid points with fanout paths')
            paths = netlist.get_all_fanout_paths(args.start_point)
            dump_path_list_report(netlist, paths, sys.stdout)
            return 0

        # Fan in paths
        if args.finish_point and not args.start_point:
            if len(args.through_points) > 0:
                raise RuntimeError('cannot specify through points with fanin paths')
            if len(args.avoid_points) > 0:
                raise RuntimeError('cannot specify avoid points with fanin paths')
            paths = netlist.get_all_fanin_paths(args.finish_point)
            dump_path_list_report(netlist, paths, sys.stdout)
            return 0

    except RuntimeError as e:
        print('Error: '+str(e))
        return 1


if __name__ == '__main__':
    sys.exit(main())
