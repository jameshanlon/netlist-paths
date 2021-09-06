# Definitions used by the tests.
configure_file(definitions.py.in
               definitions.py)

# Python wrapper tests.

configure_file(py_wrapper_tests.py
               ${CMAKE_CURRENT_BINARY_DIR}
               COPYONLY)

add_test(NAME py_wrapper_tests.py
         COMMAND ${Python_EXECUTABLE} py_wrapper_tests.py)

# Command-line tool tests.

configure_file(tool_tests.py
               ${CMAKE_CURRENT_BINARY_DIR}
               COPYONLY)

add_test(NAME tool_tests.py
         COMMAND ${Python_EXECUTABLE} tool_tests.py)
