# Definitions used by the tests.
configure_file(definitions.hpp.in
               definitions.hpp)

# Unit tests.

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
  add_test(NAME ${binary_name}
           COMMAND ./${binary_name})
endfunction()

add_test_exe(VerilatorXMLTests  VerilatorXMLTests.cpp)
add_test_exe(NameTests          NameTests.cpp)
add_test_exe(DTypeTests         DTypeTests.cpp)
add_test_exe(VertexTests        VertexTests.cpp)
add_test_exe(PathTests          PathTests.cpp)
