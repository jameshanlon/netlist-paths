configure_file(netlist_paths.py
               ${CMAKE_CURRENT_BINARY_DIR}
               COPY_ONLY)

configure_file(definitions.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/definitions.py)
