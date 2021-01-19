configure_file(netlist_paths.py
               ${CMAKE_CURRENT_BINARY_DIR}
               COPYONLY)

configure_file(definitions.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/definitions.py)

install(FILES netlist_paths.py
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/definitions.py
        DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(netlist-paths)
