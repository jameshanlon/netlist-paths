set(SOURCES
    Netlist.cpp
    RunVerilator.cpp
    ReadVerilatorXML.cpp
    Graph.cpp)

# Compile a shared library to link with the Python module since Boost
# distributions are expecting this.
add_library(netlist_paths SHARED
  ${SOURCES})
target_include_directories(netlist_paths PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(netlist_paths
  ${Boost_LIBRARIES}
  ${Python_LIBRARIES}
  ${CMAKE_DL_LIBS} # Required for Boost_DLL
  pthread)

install(TARGETS netlist_paths
        DESTINATION lib)

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

install(TARGETS py_netlist_paths
        DESTINATION lib)
