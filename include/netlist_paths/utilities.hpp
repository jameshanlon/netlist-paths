#ifndef NETLIST_PATHS_UTILITIES_HPP
#define NETLIST_PATHS_UTILITIES_HPP

#ifdef NDEBUG
#define DEBUG(x) do {} while (false)
#else
#define DEBUG(x) do { if (options.debugMode) { x; } } while (false)
#endif

#define INFO(x) do { if (options.verboseMode) { x; } } while (false)

namespace netlist_paths {

} // End namespace.

#endif // NETLIST_PATHS_UTILITIES_HPP


