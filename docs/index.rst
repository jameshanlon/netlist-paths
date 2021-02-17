Netlist paths
=============

Netlist paths is a library and command-line tool for querying a Verilog
netlist. It reads an XML representation of a design's netlist, produced by
Verilator, and provides facilities for inspecting types, variables and paths.
The library is written in C++ and has a Python interface.

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Indices and tables
==================

* :ref:`genindex`
* :ref:`search`


API reference
=============

.. doxygenclass:: netlist_paths::Netlist
   :members:

.. doxygenclass:: netlist_paths::Graph
   :members:

.. doxygenclass:: netlist_paths::Options
   :members:

.. doxygenclass:: netlist_paths::RunVerilator
   :members:

.. doxygenclass:: netlist_paths::Vertex
   :members:

.. doxygenclass:: netlist_paths::Waypoints
   :members:
