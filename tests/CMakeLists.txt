add_executable(TestCompileGraph
               compile_graph.cpp)
target_link_libraries(TestCompileGraph
                      netlist_paths
                      ${Boost_LIBRARIES})
target_include_directories(TestCompileGraph PUBLIC
                           ${CMAKE_BINARY_DIR})
add_test(NAME TestCompileGraph COMMAND TestCompileGraph)

configure_file(definitions.hpp.in
               definitions.hpp)
