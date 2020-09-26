configure_file(netlist_paths.py
               ${CMAKE_CURRENT_BINARY_DIR}
               COPY_ONLY)
configure_file(definitions.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/definitions.py)

#add_executable(netlist-paths main.cpp)
#target_link_libraries(netlist-paths
#          netlist_paths_static)
#install(TARGETS netlist-paths
#        DESTINATION bin)
