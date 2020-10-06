# Netlist paths

Netlist paths is a library and command-line tool for querying a Verilog
netlist. It reads an XML representation of a design's netlist, produced by
Verilator, and provides facilities for inspecting types, variables and paths.
The library is written in C++ and has a Python interface.


## Dependencies

- C++ compiler supporing C++14
- CMake (minimum 3.12.0)
- Boost (minimum 1.68.0)
- Python3
- make
- autoconf
- flex
- bison

Documentation:

- Doxygen
- Sphinx-doc
- ``doc/requirements.txt``

## Build
Run a debug build with Make, for example:
```
git submodule update --init --recursive
mkdir Debug
cd Debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_DOCS=ON
make -j8 install
```

## Test

Run unit tests:
```
ctest . --verbose
```

Run external tests with [netlist-paths-tests](https://github.com/jameshanlon/netlist-paths-tests).

## Debugging

Produce XML from a test case:
```
netlist_paths.py --compile tests/verilog/adder.sv --output adder.xml --verbose --debug
Running: ".../np-verilator_bin" +1800-2012ext+.sv --bbox-sys --bbox-unsup \
  --xml-only --flatten --error-limit 10000 --xml-output out.xml tests/verilog/adder.sv
Parsing input XML file
Netlist contains 15 vertices and 30 edges
```
Produce a visualisation of the netlist graph:
```
netlist-paths out.xml --verbose --debug --dotfile --outfile graph.dot
dot -Tpdf graph.dot -o graph.pdf
```
Run C++ unit tests directly:
```
cd Debug/tests
./UnitTests
```
Run Python unit tests directly (version of Python must match build):
```
cd Debug/tests
python py_wrapper_tests.py
```
