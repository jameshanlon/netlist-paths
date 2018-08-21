# Netlist paths

This is a command line tool for querying paths in a Verilog netlist.

## Building Verilator

Install dependencies: make, autoconf, g++, flex, bison. Then:
```
git submodule update --init --recursive
cd verilator
autoconf
./configure
make -j8
```

## Building the path tool

Install dependencies: CMake (minimum 3.8.0), Boost. Then:
```
mkdir build
cd build
cmake ..
make -j8
```
