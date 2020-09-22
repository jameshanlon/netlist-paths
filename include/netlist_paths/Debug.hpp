#ifndef NETLIST_PATHS_DEBUG_HPP
#define NETLIST_PATHS_DEBUG_HPP

#ifdef NDEBUG
#define DEBUG(x) do {} while (false)
#else
#define DEBUG(x) do { if (netlist_paths::Options::getInstance().debugMode) { x; } } while (false)
#endif

#define INFO(x) do { if (netlist_paths::Options::getInstance().verboseMode) { x; } } while (false)

#endif // NETLIST_PATHS_DEBUG_HPP
