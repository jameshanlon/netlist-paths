cmake_minimum_required(VERSION 3.12.0)
project(netlist-paths)
include(ExternalProject)
enable_testing()
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.68.0 REQUIRED COMPONENTS
             graph
             regex
             program_options
             system
             filesystem
             unit_test_framework)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(netlist-paths main.cpp)
target_compile_options(netlist-paths PUBLIC -Wall -pedantic -march=native)
target_link_libraries(netlist-paths
  netlist_paths
  ${Boost_LIBRARIES}
  ${CMAKE_DL_LIBS} # Required for Boost_DLL
  pthread)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # Set default install prefix.
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "..." FORCE)
endif()
install(TARGETS netlist-paths
        DESTINATION bin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(lib)
add_subdirectory(tests)
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
  ALWAYS 1)
