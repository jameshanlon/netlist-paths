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


Contributing
============

Please follow the `LLVM coding standards <https://llvm.org/docs/CodingStandards.html>`_.


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


C++ API reference
=================

Graph
-----

.. doxygenclass:: netlist_paths::Graph
   :members:

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

Waypoints
---------

.. doxygenclass:: netlist_paths::Waypoints
   :members:
