# Definitions used by the tests.
configure_file(definitions.hpp.in
               definitions.hpp)

# Unit tests.

add_executable(UnitTests
               main.cpp
               DTypeTests.cpp
               NameTests.cpp
               PathAllTests.cpp
               PathAvoidTests.cpp
               PathComponentsTests.cpp
               PathExistsTests.cpp
               PathFanInOutTests.cpp
               PathMatchAnyTests.cpp
               PathRegOutputsTests.cpp
               PathStartFinishAliasesTests.cpp
               PathThroughPointsTests.cpp
               PathThroughRegsTests.cpp
               VerilatorXMLTests.cpp
               VertexTests.cpp
               VlvboundTest.cpp
               )

target_link_libraries(UnitTests
                      netlist_paths
                      ${Boost_LIBRARIES}
                      ${Python_LIBRARIES}
                      ${CMAKE_DL_LIBS} # Required for Boost_DLL
                      pthread)

target_include_directories(UnitTests PUBLIC
                           ${CMAKE_BINARY_DIR})

add_test(NAME UnitTests
         COMMAND ./UnitTests)
