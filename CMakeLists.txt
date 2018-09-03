cmake_minimum_required(VERSION 3.8.0)
project(netlist-paths)
include(ExternalProject)
find_package(Boost COMPONENTS program_options system filesystem REQUIRED)
set(Boost_USE_STATIC_LIBS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -march=native")
include_directories(${Boost_INCLUDE_DIRS})
add_executable(netlist-paths main.cpp)
target_link_libraries(netlist-paths
  ${Boost_LIBRARIES}
  pthread)
set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install)
install(TARGETS netlist-paths
  DESTINATION ${INSTALL_DIR}/bin)
ExternalProject_Add(Verilator
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/verilator
  PREFIX verilator
  CONFIGURE_COMMAND autoconf && ./configure --prefix=${INSTALL_DIR}
  BUILD_COMMAND make
  BUILD_IN_SOURCE 1
  BUILD_ALWAYS 1
  INSTALL_COMMAND make install)
