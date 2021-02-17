![Build](https://github.com/jameshanlon/netlist-paths/workflows/Build%20and%20test/badge.svg)
[![Documentation Status](https://readthedocs.org/projects/netlist-paths/badge/?version=latest)](https://netlist-paths.readthedocs.io/en/latest/?badge=latest)

# Netlist paths

Netlist paths is a library and command-line tool for querying a Verilog
netlist. It reads an XML representation of a design's netlist, produced by
Verilator, and provides facilities for inspecting types, variables and paths.
The library is written in C++ and has a Python interface.


## Build and installation

The following dependencies must be installed:

- C++ compiler supporing C++14
- CMake (minimum 3.12.0)
- Boost (minimum 1.65.0)
- Python3
- Make
- Autoconf
- Flex
- Bison

The following are additional dependencies are required to build the documentation:

- Doxygen
- Sphinx-doc
- Python packages listed in  ``doc/requirements.txt``

To build and install netlist paths, configure the build system with CMake, then run Make:
```
git submodule update --init --recursive
mkdir Release
cd Release
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8 install
```

Optionally, run the unit tests:
```
ctest .
```


## Example use of the command-line tool

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

## License

Netlist Paths is licensed under the Apache 2.0 license. See [LICENSE](LICENSE) for details.
