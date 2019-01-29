# Unit tests

function(add_test_exe binary_name source_files)
  add_executable(${binary_name}
                 ${source_files})
  target_link_libraries(${binary_name}
                        netlist_paths
                        ${Boost_LIBRARIES}
                        ${CMAKE_DL_LIBS} # Required for Boost_DLL
                        pthread)
  target_include_directories(${binary_name} PUBLIC
                             ${CMAKE_BINARY_DIR})
  add_test(NAME ${binary_name} COMMAND ./${binary_name})
endfunction()

configure_file(definitions.hpp.in
               definitions.hpp)

add_test_exe(TestCompileGraph compile_graph.cpp)
add_test_exe(TestAnalyseGraph analyse_graph.cpp)

# Pytest

configure_file(basic_test.py
	       ${CMAKE_CURRENT_BINARY_DIR}
	       COPYONLY)

configure_file(definitions.py.in
	       definitions.py)
