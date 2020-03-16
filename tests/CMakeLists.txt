# Definitions used by the tests.
configure_file(definitions.hpp.in
               definitions.hpp)
configure_file(definitions.py.in
               definitions.py)

# Unit tests.

function(add_test_exe binary_name source_files)
  add_executable(${binary_name}
                 ${source_files})
  target_link_libraries(${binary_name}
                        netlist_paths_shared
                        ${Boost_LIBRARIES}
                        ${Python_LIBRARIES}
                        ${CMAKE_DL_LIBS} # Required for Boost_DLL
                        pthread)
  target_include_directories(${binary_name} PUBLIC
                             ${CMAKE_BINARY_DIR})
  add_test(NAME ${binary_name}
           COMMAND ./${binary_name})
endfunction()

add_test_exe(TestBasicModules basic_modules.cpp)

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

