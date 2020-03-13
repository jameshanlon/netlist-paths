# netlist-paths library
add_library(netlist_paths SHARED
  CompileGraph.cpp
  AnalyseGraph.cpp
  Options.cpp)
target_include_directories(netlist_paths PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(netlist_paths
  ${Boost_LIBRARIES}
  ${CMAKE_DL_LIBS} # Required for Boost_DLL
  pthread)

add_library(netlist_paths_static STATIC
  CompileGraph.cpp
  AnalyseGraph.cpp
  Options.cpp)
target_include_directories(netlist_paths_static PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(netlist_paths_static
  ${Boost_LIBRARIES}
  ${Python_LIBRARIES}
  ${CMAKE_DL_LIBS} # Required for Boost_DLL
  pthread)

# Python wrapper module
add_library(py_netlist_paths MODULE
    python_wrapper.cpp)
target_link_libraries(py_netlist_paths
    netlist_paths
    ${Python_LIBRARIES})
target_include_directories(py_netlist_paths PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${Python_INCLUDE_DIRS})
set_target_properties(py_netlist_paths PROPERTIES PREFIX "")
