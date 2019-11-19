# Check arguments.
foreach(var EXECUTABLE_INSTALL_PREFIX)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "'${var}' must be defined on the command line")
  endif()
endforeach()
# Perfom the renaming.
file(GLOB installed_executables ${EXECUTABLE_INSTALL_PREFIX}/verilator*)
foreach (executable ${installed_executables})
  get_filename_component(executable_filename ${executable} NAME)
  string(REPLACE "verilator" "np-verilator"
                 new_executable_filename
                 ${executable_filename})
  file(RENAME ${EXECUTABLE_INSTALL_PREFIX}/${executable_filename}
              ${EXECUTABLE_INSTALL_PREFIX}/${new_executable_filename})
  message(STATUS "Renamed ${executable_filename} to ${new_executable_filename}")
endforeach()
