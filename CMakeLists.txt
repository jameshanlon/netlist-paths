cmake_minimum_required(VERSION 3.8.0)
project(netlist-paths)
find_package(Boost COMPONENTS program_options system filesystem REQUIRED)
set(Boost_USE_STATIC_LIBS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -march=native")
include_directories(${Boost_INCLUDE_DIRS})
add_executable(netlist-paths main.cpp)
target_link_libraries(netlist-paths
  ${Boost_LIBRARIES}
  pthread)
