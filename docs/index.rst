Netlist Paths
=============

Netlist Paths is a library and command-line tool for querying a Verilog
netlist. It reads an XML representation of a design's netlist, produced by
`Verilator <https://www.veripool.org/wiki/verilator>`_, and provides facilities
for inspecting variables and their type and dependency information. The library
is written in C++ and has a Python interface allowing it to be integrated
easily into scripts.

Compared with standard front-end EDA tools such as Synopsys Verdi and Spyglass,
Netlist Paths is oriented towards command-line use for exploration of a design
(rather than with a GUI), and for integration with Python infrastructure
(rather than TCL) to build tools for analysing or debugging a design.

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Indices and tables
==================

* :ref:`genindex`
* :ref:`search`


Build and installation
======================

The following dependencies must be installed:

- C++ compiler supporting C++14
- CMake (minimum 3.12.0)
- Boost (minimum 1.65.0)
- Python 3.8
- Make
- Autoconf
- Flex
- Bison
- Doxygen (only if building the documentation)

To build and install netlist paths, configure the build system with CMake, then
run Make:

.. code-block:: bash

  ➜ git submodule update --init --recursive
  ...
  ➜ mkdir Release
  ➜ cd Release
  ➜ cmake .. -DCMAKE_BUILD_TYPE=Release
  ...
  ➜ make -j8 install
  ...

Optionally, run the unit tests:

.. code-block:: bash

  ➜ ctest
  ...

To build the documentation add ``-DNETLIST_PATHS_BUILD_DOCS=1`` to the ``cmake``
command.

Set ``PATH`` and ``PYTHONPATH`` appropriately to the ``bin`` and ``lib``
directories of the installation to make the command-line tools and Python
modules accessible.


Getting started
===============

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
respectively.

Note that the ``--compile`` flag causes Verilator to be run to create the XML
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


Contributing
============

Contributions are welcome, please follow the `LLVM coding standards <https://llvm.org/docs/CodingStandards.html>`_.


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

Options
-------

.. doxygenclass:: netlist_paths::Options
   :members:

Netlist
-------

.. doxygenclass:: netlist_paths::Netlist
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

