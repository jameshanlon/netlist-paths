add_executable(netlist-paths main.cpp)

target_link_libraries(netlist-paths
                      netlist_paths
                      ${Boost_LIBRARIES}
                      ${CMAKE_DL_LIBS} # Required for Boost_DLL
                      pthread)

install(TARGETS netlist-paths
        DESTINATION bin)
