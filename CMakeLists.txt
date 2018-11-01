cmake_minimum_required(VERSION 3.12.0)
project(netlist-paths)
include(ExternalProject)
enable_testing()
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS
             program_options
             system
             filesystem
             unit_test_framework
             REQUIRED)
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
set(INSTALL_DIR ${CMAKE_BINARY_DIR}/install)
install(TARGETS netlist-paths
  DESTINATION ${INSTALL_DIR}/bin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(lib)
add_subdirectory(tests)
# Verilator
ExternalProject_Add(Verilator
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/verilator
  PREFIX verilator
  CONFIGURE_COMMAND autoconf && ./configure --prefix=${INSTALL_DIR}
  BUILD_COMMAND make
  BUILD_IN_SOURCE 1
  BUILD_ALWAYS 1
  INSTALL_COMMAND make install)
