add_executable(netlist-paths main.cpp)
target_link_libraries(netlist-paths
                      netlist_paths_static)
install(TARGETS netlist-paths
        DESTINATION bin)
