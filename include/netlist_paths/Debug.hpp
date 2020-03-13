#ifndef NETLIST_PATHS_DEBUG_HPP
#define NETLIST_PATHS_DEBUG_HPP

#ifdef NDEBUG
#define DEBUG(x) do {} while (false)
#else
#define DEBUG(x) do { if (netlist_paths::options.debugMode) { x; } } while (false)
#endif

#define INFO(x) do { if (netlist_paths::options.verboseMode) { x; } } while (false)

#endif // NETLIST_PATHS_DEBUG_HPP
