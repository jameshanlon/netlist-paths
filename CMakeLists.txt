cmake_minimum_required(VERSION 3.12.0)
project(netlist-paths)
include(ExternalProject)
include(GNUInstallDirs)

option(NETLIST_PATHS_BUILD_DOCS "Create and install HTML documentation" OFF)
option(NETLIST_PATHS_INCLUDE_TESTS "Include test targets in the build" ON)

set(Boost_USE_MULTITHREADED ON)

set(NETLIST_PATHS_VERSION_MAJOR 0)
set(NETLIST_PATHS_VERSION_MINOR 2)

# Use local find scripts
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(Python REQUIRED COMPONENTS
             Interpreter
             Development)

find_package(Boost 1.65.0 REQUIRED COMPONENTS
             graph
             regex
             program_options
             system
             filesystem
             python
             unit_test_framework
             log
             log_setup)

message(STATUS "Python_LIBRARIES    = ${Python_LIBRARIES}")
message(STATUS "Python_EXECUTABLE   = ${Python_EXECUTABLE}")
message(STATUS "Python_INCLUDE_DIRS = ${Python_INCLUDE_DIRS}")
message(STATUS "Boost_INCLUDE_DIRS  = ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES     = ${Boost_LIBRARIES}")

# CXX compiler
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -pedantic)
add_definitions(-DBOOST_LOG_DYN_LINK) # Dynamic link option for boost::log

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # Set default install prefix.
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "..." FORCE)
endif()

include_directories(${Boost_INCLUDE_DIRS}
                    ${Python_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include)

# Set the install RPATH
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

# Verilator
ExternalProject_Add(Verilator
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/verilator
  PREFIX verilator
  CONFIGURE_COMMAND autoconf && ./configure --prefix=<INSTALL_DIR>
  BUILD_COMMAND make
  BUILD_IN_SOURCE 1
  BUILD_ALWAYS 1
  INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
  INSTALL_COMMAND make install)

# Rename Verilator binaries to differentiate from other installed versions.
ExternalProject_Add_Step(Verilator add_executable_prefix
  COMMAND ${CMAKE_COMMAND}
            -DEXECUTABLE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/bin
            -P ${CMAKE_CURRENT_SOURCE_DIR}/add_executable_prefix.cmake
  COMMENT "Adding Verilator executable prefix"
  DEPENDEES mkdir update patch download configure build install
  ALWAYS 1)

add_subdirectory(lib)
add_subdirectory(tools)

if (NETLIST_PATHS_INCLUDE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if (NETLIST_PATHS_BUILD_DOCS)
  add_subdirectory(docs)
endif()
