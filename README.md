![Build](https://github.com/jameshanlon/netlist-paths/workflows/Build%20and%20test/badge.svg)
[![Documentation Status](https://readthedocs.org/projects/netlist-paths/badge/?version=latest)](https://netlist-paths.readthedocs.io/en/latest/?badge=latest)

# Netlist paths

Netlist paths is a library and command-line tool for querying a Verilog
netlist. It reads an XML representation of a design's netlist, produced by
Verilator, and provides facilities for inspecting types, variables and paths.
The library is written in C++ and has a Python interface.


## Dependencies

- C++ compiler supporing C++14
- CMake (minimum 3.12.0)
- Boost (minimum 1.65.0)
- Python3
- Make
- Autoconf
- Flex
- Bison

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
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j8 install
```

Run the unit tests:
```
ctest .
```


## Example use

List all the named entities in a design:
```
$ netlist-paths tests/verilog/fsm.sv --compile --dump-names
Name           Type DType        Width Direction Location
-------------- ---- ------------ ----- --------- -----------------------
fsm.i_clk      VAR  logic        1     INPUT     tests/verilog/fsm.sv:3
fsm.i_finish   VAR  logic        1     INPUT     tests/verilog/fsm.sv:6
fsm.i_rst      VAR  logic        1     INPUT     tests/verilog/fsm.sv:4
fsm.i_start    VAR  logic        1     INPUT     tests/verilog/fsm.sv:5
fsm.i_wait     VAR  logic        1     INPUT     tests/verilog/fsm.sv:7
fsm.next_state VAR  packed union 3     NONE      tests/verilog/fsm.sv:31
fsm.o_state    VAR  [2:0] logic  3     OUTPUT    tests/verilog/fsm.sv:8
fsm.state_q    REG  packed union 3     NONE      tests/verilog/fsm.sv:28
i_clk          VAR  logic        1     INPUT     tests/verilog/fsm.sv:3
i_finish       VAR  logic        1     INPUT     tests/verilog/fsm.sv:6
i_rst          VAR  logic        1     INPUT     tests/verilog/fsm.sv:4
i_start        VAR  logic        1     INPUT     tests/verilog/fsm.sv:5
i_wait         VAR  logic        1     INPUT     tests/verilog/fsm.sv:7
o_state        VAR  [2:0] logic  3     OUTPUT    tests/verilog/fsm.sv:8
```

Or, just list the registrers:
```
$ netlist-paths tests/verilog/fsm.sv --compile --dump-regs
Name        Type DType        Width Direction Location
----------- ---- ------------ ----- --------- -----------------------
fsm.state_q REG  packed union 3     NONE      tests/verilog/fsm.sv:28
```

Report the components of a path between two points, in this case between a
register and an output port:
```
$ netlist-paths tests/verilog/fsm.sv --compile --from fsm.state_q --to o_state

Name        Type         Statement Location
----------- ------------ --------- -----------------------
fsm.state_q packed union ASSIGN    tests/verilog/fsm.sv:55
o_state     [2:0] logic
```

List paths starting from a particular register:
```
$ netlist-paths tests/verilog/fsm.sv --compile --from fsm.state_q

Path 0
Name        Type         Statement Location
----------- ------------ --------- -----------------------
fsm.state_q packed union ASSIGN    tests/verilog/fsm.sv:55
o_state     [2:0] logic

Path 1
Name        Type         Statement Location
----------- ------------ --------- -----------------------
fsm.state_q packed union ASSIGN    tests/verilog/fsm.sv:55
o_state     [2:0] logic
fsm.o_state [2:0] logic

Path 2
Name           Type         Statement Location
-------------- ------------ --------- -----------------------
fsm.state_q    packed union ASSIGN    tests/verilog/fsm.sv:34
fsm.next_state packed union ASSIGN    tests/verilog/fsm.sv:51
fsm.state_q    packed union
```

## Extended tests

To run the extended test set, the
[netlist-paths-tests](https://github.com/jameshanlon/netlist-paths-tests)
repository contains tests based on external System Verilog designs.

## Debugging

Produce XML from a test case:
```
$ netlist-paths --compile tests/verilog/adder.sv -o adder.xml --verbose
info: Running "/Users/jamieh/netlist-paths/Debug/install/bin/np-verilator_bin" +1800-2012ext+.sv --bbox-sys --bbox-unsup --xml-only --flatten --error-limit 10000 --xml-output adder.xml tests/verilog/adder.sv
info: Parsing input XML file
info: 1 modules in netlist
info: 0 interfaces in netlist
info: 0 packages in netlist
info: 4 entries in type table
info: Netlist contains 15 vertices and 22 edges
```
Produce a visualisation of the netlist graph:
```
$ netlist-paths adder.xml --dump-dot -o adder.dot
$ dot -Tpdf adder.dot -o adder.pdf
```

Run C++ unit tests directly, eg:
```
$ cd Debug
$ ./tests/NameTests
```
Run Python unit tests directly (version of Python must match build):
```
$ cd Debug/tests
$ python3 py_wrapper_tests.py
```
