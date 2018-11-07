# Netlist paths

This is a command line tool for querying paths in a Verilog netlist.

## Compile and install

Install dependencies:
```
C/C++ compiler
CMake (minimum 3.12.0)
Boost
```
Install Verilator dependencies:
```
make
autoconf
flex
bison
```
Then:
```
git submodule update --init --recursive
mkdir Debug
cd Debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j8 install
ctest
```
