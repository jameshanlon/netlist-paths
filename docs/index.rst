Netlist Paths
=============

Netlist Paths is a library and command-line tool for querying a Verilog
netlist. It reads an XML representation of a design's netlist, produced by
`Verilator <https://www.veripool.org/wiki/verilator>`_, and provides facilities
for inspecting variables and their data type and dependency information. The
library is written in C++ and has a Python interface allowing it to be
integrated easily into scripts.

Compared with standard front-end EDA tools such as Synopsys Verdi and Spyglass,
Netlist Paths is oriented towards command-line use for exploration of a design
(rather than with a GUI), and for integration with Python infrastructure
(rather than TCL) to build tools for analysing or debugging a design. By
focusing on source-level connectivity it is lightweight and will run faster
than standard tools to perform a comparable task, whilst also being open source
and unrestricted by licensing issues. Applications include critical timing path
investigation, creation of unit tests for design structure and connectivity,
and development of patterns for quality-of-result reporting.

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Installation
============

The following dependencies must be installed:

- C++ compiler supporting C++17
- CMake (minimum 3.12.0)
- Boost (minimum 1.65.0)
- Python (minimum 3.8)
- Make
- Autoconf
- Flex
- Bison
- Doxygen (only required if building the documentation)

To build and install netlist paths, configure the build system with CMake, then
run Make. Note that Verilator is included as a submodule and as part of the build.

.. code-block:: bash

  ➜ git submodule update --init --recursive
  ...
  ➜ mkdir Release
  ➜ cd Release
  ➜ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./install
  ...
  ➜ make -j8 install
  ...

Optionally, run the unit tests:

.. code-block:: bash

  ➜ ctest --verbose
  ...

To build the documentation add ``-DNETLIST_PATHS_BUILD_DOCS=1`` to the
``cmake`` command, and before running the ``cmake`` step, install the
dependencies in a virtualenv:

.. code-block:: bash

  ➜ cd Release
  ➜ virtualenv -p python3 env
  ...
  ➜ source env/bin/activate
  ➜ pip install -r ../docs/requirements.txt

Once the build and install steps have completed, set ``PATH`` and
``PYTHONPATH`` appropriately to the ``bin`` and ``lib`` directories of the
installation to make the command-line tools and Python modules accessible.


User guide
==========

Command-line tool
-----------------

The easiest way to use Netlist Paths is with the ``netlist-paths`` command
line tool, which provides ways to access the library's functionalities. For
example (assuming ``PATH`` includes your installation directory), to compile
and view a list of named objects in the FSM example module:

.. code-block:: bash

  ➜ netlist-paths --compile examples/fsm.sv --dump-names
  Name           Type DType        Width Direction Location
  -------------- ---- ------------ ----- --------- ------------------
  fsm.i_clk      VAR  logic        1     INPUT     examples/fsm.sv:3
  fsm.i_finish   VAR  logic        1     INPUT     examples/fsm.sv:6
  fsm.i_rst      VAR  logic        1     INPUT     examples/fsm.sv:4
  fsm.i_start    VAR  logic        1     INPUT     examples/fsm.sv:5
  fsm.i_wait     VAR  logic        1     INPUT     examples/fsm.sv:7
  fsm.next_state VAR  packed union 3     NONE      examples/fsm.sv:31
  fsm.o_state    VAR  [2:0] logic  3     OUTPUT    examples/fsm.sv:8
  fsm.state_q    REG  packed union 3     NONE      examples/fsm.sv:28
  i_clk          VAR  logic        1     INPUT     examples/fsm.sv:3
  i_finish       VAR  logic        1     INPUT     examples/fsm.sv:6
  i_rst          VAR  logic        1     INPUT     examples/fsm.sv:4
  i_start        VAR  logic        1     INPUT     examples/fsm.sv:5
  i_wait         VAR  logic        1     INPUT     examples/fsm.sv:7
  o_state        VAR  [2:0] logic  3     OUTPUT    examples/fsm.sv:8

This output lists each of the variables in the design, their type (variable or
register), the Verilog data type, with data type width, the direction of the
variable (ports only) and the source location. This list can be filtered by
also supplying a pattern (with the ``--regex`` flag to enable regular
expression matching):

.. code-block:: bash

  ➜ netlist-paths --compile examples/fsm.sv --dump-names state --regex
  Name           Type DType        Width Direction Location
  -------------- ---- ------------ ----- --------- ------------------
  fsm.next_state VAR  packed union 3     NONE      examples/fsm.sv:31
  fsm.o_state    VAR  [2:0] logic  3     OUTPUT    examples/fsm.sv:8
  fsm.state_q    REG  packed union 3     NONE      examples/fsm.sv:28
  o_state        VAR  [2:0] logic  3     OUTPUT    examples/fsm.sv:8

There is similar behaviour with ``--dump-nets``, ``--dump-ports``,
``--dump-regs`` to select only net, port or register variable types
respectively. The argument ``--dump-dtypes`` reports the named data types
used in the design:

.. code-block:: bash

  ➜ netlist-paths --compile examples/fsm.sv --dump-dtypes fsm --regex
  Name             Width Description
  ---------------- ----- ------------
  fsm.state_enum_t 3     enum
  fsm.state_t      3     packed union

Note that the ``--compile`` argument causes Verilator to be run to create the XML
netlist, and is useful for compiling simple examples. Execution of Verilator
can be seen with verbose output:

.. code-block:: bash

  ➜ netlist-paths --compile examples/fsm.sv --verbose
  info: Running ".../bin/np-verilator_bin" +1800-2012ext+.sv --bbox-sys --bbox-unsup
    --xml-only --flatten --error-limit 10000 --xml-output l9pvjzq6 examples/fsm.sv
  ...

The ``-I`` and ``-D`` arguments can be used with ``--compile`` to add include
directories and macro definitions respectively for Verilator, but for more
complex invocations, Verilator can just be run separately and the path to the
XML output provided to ``netlist-paths`` as an argument.

Command-line tool reference
---------------------------

.. code-block:: bash

  usage: netlist-paths [-h] [-c] [-I include_path] [-D definition] [-o file] [--dump-names [pattern]]
                       [--dump-nets [pattern]] [--dump-ports [pattern]] [--dump-regs [pattern]]
                       [--dump-dtypes [pattern]] [--dump-dot] [--from point] [--to point] [--through point]
                       [--avoid point] [--traverse-registers] [--start-anywhere] [--end-anywhere] [--all-paths]
                       [--regex] [--wildcard] [--ignore-hierarchy-markers] [-v] [-d]
                       files [files ...]

  Query a Verilog netlist

  positional arguments:
    files                       Input files

  options:
    -h, --help                  Show this help message and exit
    -c, --compile               Run Verilator to compile a netlist
    -I include_path             Add an source include path (only with --compile)
    -D definition               Define a preprocessor macro (only with --compile)
    -o file,--output file       Specify an output file
    --dump-names [pattern]      Dump all named entities, filter by regex
    --dump-nets [pattern]       Dump all nets, filter by regex
    --dump-ports [pattern]      Dump all ports, filter by regex
    --dump-regs [pattern]       Dump all registers, filter by regex
    --dump-dtypes [pattern]     Dump all data types, filter by regex
    --dump-dot                  Dump a dotfile of the netlist's graph
    --from point                Specify a path start point
    --to point                  Specify a path finish point
    --through point             Specify a path though point
    --avoid point               Specify a point for a path to avoid
    --traverse-registers        Allow paths to traverse registers
    --start-anywhere            Allow paths to start on any variable
    --end-anywhere              Allow paths to end on any variable
    --all-paths                 Find all paths between two points (exponential time)
    --regex                     Enable regular expression matching of names
    --wildcard                  Enable wildcard matching of names
    --ignore-hierarchy-markers  Ignore hierarchy markers: _ . /
    -v, --verbose               Print execution information
    -d, --debug                 Print debugging information


Python module
-------------

The functionality of Netlist Paths can be accessed in Python, using the
``py_netlist_paths`` module, which is is a wrapper around the C++ library. Full
details of the API can be found in :ref:`python_api`, and for the C++ library
in :ref:`cpp_api`. As a simple example, the Python Netlist Paths library can be
used interactively, first by creating an XML netlist file (and making sure the
library install directory is a search path for Python modules:

.. code-block:: bash

  ➜ netlist-paths --compile examples/fsm.sv --output fsm.xml
  ➜ export PYTHONPATH=/path/to/netlist-paths/install/lib:$PYTHONPATH

A new ``Netlist`` object can then be created, specifying the XML file as an
argument, and the list of named objects can be retrieved as above using
the ``get_named_vertices()`` method:

.. code-block:: bash

  ➜ python3
  Python 3.9.0 (default, Dec  6 2020, 18:02:34)
  [Clang 12.0.0 (clang-1200.0.32.27)] on darwin
  Type "help", "copyright", "credits" or "license" for more information.
  >>> from py_netlist_paths import Netlist
  >>> netlist = Netlist('fsm.xml')
  >>> for v in netlist.get_reg_vertices():
  ...     print(v.get_name())
  ...
  fsm.state_q

This can then easily be turned into a script to create a tool that reports
registers in a design:

.. code-block:: python

  import argparse
  import sys
  from py_netlist_paths import Netlist

  if __name__ == '__main__':
      parser = argparse.ArgumentParser()
      parser.add_argument('netlist_file')
      args = parser.parse_args()
      netlist = np.Netlist(args.netlist_file)
      for register in netlist.get_reg_vertices():
          print('{} {}'.format(register.get_name(), register.get_dtype_str()))

This example tool can be run from the examples directory:

.. code-block:: bash

  ➜ netlist-paths --compile examples/fsm.sv --output fsm.xml
  ➜ python3 -m examples.list_registers -h
  usage: list_registers.py [-h] netlist_file

  positional arguments:
    netlist_file

  optional arguments:
    -h, --help    show this help message and exit
  ➜ python3 -m examples.list_registers fsm.xml
  fsm.state_q packed union


Running Netlist Paths on a design
---------------------------------

If it is possible to elaborate a design with Verilator, then Netlist Paths can
be used to analyse the XML netlist of that design. Verilator produces the
netlist with the arguments ``--xml-only`` (causing it to only create XML
output) and ``--flatten`` (causing it to force inlining of all modules, tasks
and functions, fully elaborating the netlist). Examples of complex, real-world
designs can be found in the `netlist-paths-tests
<https://github.com/jameshanlon/netlist-paths-tests>`_ repository.


Limitations
-----------

Netlist Paths has some limitations that should be considered when using it:

- It does not perform elaboration of the design at the bit level,
  meaning that dependencies are between named variables, rather than components
  of variables, such as subscripts, slices, or fields. As such, the
  dependencies it does infer are at a coarse level of granularity. Full
  bit-level elaboration (or bit blasting) will substantially impact the size of
  the netlist graph, and the corresponding runtimes, but it is something that may
  be supported in the future.

- During elaboration of the design, Verilator introduces new entities that do
  not appear in the original design source, with names beginning or
  containing ``__V``. This is a artefact of Verilator preparing the syntax
  tree for emission as C++ code for simulation, rather than as a true
  source-level netlist. Future releases of Verilator may improve this mapping.


Contributing
============

Contributions are welcome, check the
`GitHub issues page <https://github.com/jameshanlon/netlist-paths/issues>`_
for work to do, and
please follow the
`LLVM coding standards <https://llvm.org/docs/CodingStandards.html>`_.


Developer notes
---------------

The command-line flags ``--verbose`` and ``--debug`` provide logging
information that can aid debugging.

To produce XML from a test case (noting the underlying call to Verilator):

.. code-block:: bash

  ➜ cat tests/verilog/adder.sv
  module adder
    #(parameter p_width = 32)(
      input  logic [p_width-1:0] i_a,
      input  logic [p_width-1:0] i_b,
      output logic [p_width-1:0] o_sum,
      output logic               o_co
    );
    assign {o_co, o_sum} = i_a + i_b;
  endmodule
  ➜ netlist-paths --compile tests/verilog/adder.sv -o adder.xml --verbose
  info: Running ".../netlist-paths/Debug/install/bin/np-verilator_bin" +1800-2012ext+.sv
     --bbox-sys --bbox-unsup --xml-only --flatten --error-limit 10000 --xml-output adder.xml tests/verilog/adder.sv
  info: Parsing input XML file
  info: 1 modules in netlist
  info: 0 interfaces in netlist
  info: 0 packages in netlist
  info: 4 entries in type table
  info: Netlist contains 15 vertices and 22 edges

To produce a visualisation of the netlist graph, a dot file can be produced and
rendered into an image. This can be useful to understand the structure of the
graph, although it is only practical to use with small designs.

.. code-block:: bash

  ➜ netlist-paths adder.xml --dump-dot -o adder.dot
  ➜ dot -Tpdf adder.dot -o adder.pdf

.. figure:: images/adder-graph.*

The tests are split into three categories: C++ unit tests for the library
components, Python unit tests for the Python API and Python integration tests
for the command-line tools. These can all be run automatically using `ctest`,
or individually.

.. code-block:: bash

  ➜ ctest --verbose # Run all tests from the build directory.

For the C++ unit tests:

.. code-block:: bash

  ➜ ./tests/unit/UnitTests --help
  ➜ ./tests/unit/UnitTests --list_content # Report a list of all the tests.
  ➜ ./tests/unit/UnitTests # Run all the unit tests
  ...

For the Python API unit tests:

.. code-block:: bash

  ➜ cd Debug/tests/integration
  ➜ python3 -m unittest py_wrapper_tests.py
  ...

For the Python integration tests:

.. code-block:: bash

  ➜ cd Debug/tests/integration
  ➜ python3 -m unittest tool_tests.py
  ...

To run the extended test set, the
`netlist-paths-tests <https://github.com/jameshanlon/netlist-paths-tests>`_
repository contains tests based on external System Verilog designs.

Internals
---------

Netlist Paths works by reading an XML representation of a Verilog design
produced by Verilator, which is a single flattened module with all tasks,
functions, and sub-module instances inlined (Verilator is run using the
``--xml-only`` and ``--flatten`` options). The XML is traversed to build up
a directed graph data structure representing the design netlist, where nodes of
the graph are logic statements or variables, and edges are data dependencies
between them. The data-type information of the design is also read from the
XML, and a corresponding data-type table is constructed, that each variable
references.

To support querying of paths that start and/or finish on registers (as is the case
with physical timing reports), registers are identified during parsing of the
XML so they can be split into two components: a source and a destination, with
out-edges and in-edges of the original node respectively. Register variables
are identified when they appear on the left-hand side of a non-blocking
assignment [#register-identification]_. As an example, consider the following
module that provides a bank of asynchronous-reset flip flops:

.. code-block:: verilog

  module pipestage
    #(parameter p_width=32) (
      input  logic               i_clk,
      input  logic               i_rst,
      input  logic [p_width-1:0] in,
      output logic [p_width-1:0] out
    );
    logic [p_width-1:0] data_q;
    always_ff @(posedge i_clk or posedge i_rst)
      if (i_rst) begin
        data_q <= '0;
      end else begin
        data_q <= in;
      end
    assign out = data_q;
  endmodule

The netlist graph of the ``pipestage`` has two distinct components: the fan
in-cone to ``data_q`` (appearing as a ``DST_REG``) including the sensitivity
list of the always block and the input port, and ``data_q`` driving the output
port (appearing as a ``SRC_REG``).

.. figure:: images/pipestage-graph.*

Edges between register endpoints and subsequent uses of the register are
retained in the graph to support queries 'through' registers, which can be
useful for establishing connectivity in a pipelined design.

.. [#register-identification] Note that this condition is simplistic and will
   be extended to check the always block is sensitive to a rising or falling
   edge of a clock. It is however sufficient to identify registers in most RTL
   code, which generally does not mix blocking and non-blocking assignments.
   See `Issue 5 <https://github.com/jameshanlon/netlist-paths/issues/5>`_.

.. _cpp_api:

C++ API reference
=================

DType
-----

.. doxygenclass:: netlist_paths::DType
   :members:

Exception
---------

.. doxygenclass:: netlist_paths::Exception

.. doxygenclass:: netlist_paths::XMLException

Netlist
-------

.. doxygenclass:: netlist_paths::Netlist
   :members:

Options
-------

.. doxygenclass:: netlist_paths::Options
   :members:

Path
-----

.. doxygenclass:: netlist_paths::Path
  :members:

RunVerilator
------------

.. doxygenclass:: netlist_paths::RunVerilator
   :members:

Vertex
------

.. doxygenclass:: netlist_paths::Vertex
   :members:

.. doxygenenum:: netlist_paths::VertexAstType

.. doxygenenum:: netlist_paths::VertexNetlistType

.. doxygenenum:: netlist_paths::VertexDirection

Waypoints
---------

.. doxygenclass:: netlist_paths::Waypoints
   :members:


.. _python_api:

Python API reference
====================

DType
-----

.. autoclass:: py_netlist_paths.DType
   :members:
   :undoc-members:

Options
-------

.. autoclass:: py_netlist_paths.Options
   :members:
   :undoc-members:

Netlist
-------

.. autoclass:: py_netlist_paths.Netlist
   :members:
   :undoc-members:

Waypoints
---------

.. autoclass:: py_netlist_paths.Waypoints
   :members:
   :undoc-members:

Path
----

.. autoclass:: py_netlist_paths.Path
   :members:
   :undoc-members:

RunVerilator
------------

.. autoclass:: py_netlist_paths.RunVerilator
   :members:
   :undoc-members:

Vertex
------

.. autoclass:: py_netlist_paths.Vertex
   :members:
   :undoc-members:

