add_library(netlist_paths
  CompileGraph.cpp
  AnalyseGraph.cpp)
target_include_directories(netlist_paths PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
