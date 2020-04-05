cmake_minimum_required(VERSION 3.12.0)
project(netlist-paths)
include(ExternalProject)
#set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Python REQUIRED COMPONENTS
             Interpreter
             Development)
find_package(Boost 1.68.0 REQUIRED COMPONENTS
             graph
             regex
             program_options
             system
             filesystem
             python
             unit_test_framework)
message(STATUS "Python_LIBRARIES    = ${Python_LIBRARIES}")
message(STATUS "Python_EXECUTABLE   = ${Python_EXECUTABLE}")
message(STATUS "Python_INCLUDE_DIRS = ${Python_INCLUDE_DIRS}")
message(STATUS "Boost_INCLUDE_DIRS  = ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES     = ${Boost_LIBRARIES}")

# CXX compiler
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -pedantic -march=native)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # Set default install prefix.
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "..." FORCE)
endif()

include_directories(${Boost_INCLUDE_DIRS}
                    ${Python_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include)

enable_testing()

# Verilator
ExternalProject_Add(Verilator
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/verilator
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
add_subdirectory(tests)
