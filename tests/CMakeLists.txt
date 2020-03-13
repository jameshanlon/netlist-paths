# Unit tests

function(add_test_exe binary_name source_files)
  add_executable(${binary_name}
                 ${source_files})
  target_link_libraries(${binary_name}
                        netlist_paths
                        ${Boost_LIBRARIES}
                        ${Python_LIBRARIES}
                        ${CMAKE_DL_LIBS} # Required for Boost_DLL
                        pthread)
  target_include_directories(${binary_name} PUBLIC
                             ${CMAKE_BINARY_DIR})
  add_test(NAME ${binary_name} COMMAND ./${binary_name})
endfunction()

configure_file(definitions.hpp.in
               definitions.hpp)

add_test_exe(TestCompileGraph compile_graph.cpp)

# Python wrapper

file(COPY py_wrapper_tests.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME py_wrapper_test.py
         COMMAND ${Python_EXECUTABLE} py_wrapper_tests.py)

# Pytest

configure_file(integration_tests.py
               ${CMAKE_CURRENT_BINARY_DIR}
               COPYONLY)

configure_file(definitions.py.in
               definitions.py)
