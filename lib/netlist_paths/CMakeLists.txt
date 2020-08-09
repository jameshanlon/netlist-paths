set(SOURCES
    NetlistPaths.cpp
    RunVerilator.cpp
    ReadVerilatorXML.cpp
    Netlist.cpp
    Options.cpp)

# Compile a shared library to link with the Python module since Boost
# distributions are expecting this.
add_library(netlist_paths_shared SHARED
  ${SOURCES})
target_include_directories(netlist_paths_shared PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(netlist_paths_shared
  ${Boost_LIBRARIES}
  ${Python_LIBRARIES}
  ${CMAKE_DL_LIBS} # Required for Boost_DLL
  pthread)

install(TARGETS netlist_paths_shared
        DESTINATION lib)

# Compile a static version of the library to link with the command-line tool.
add_library(netlist_paths_static STATIC
  ${SOURCES})
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
    netlist_paths_shared
    ${Python_LIBRARIES})
target_include_directories(py_netlist_paths PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${Python_INCLUDE_DIRS})
set_target_properties(py_netlist_paths PROPERTIES PREFIX "")

install(TARGETS py_netlist_paths
        DESTINATION lib)
