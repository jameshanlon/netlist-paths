#ifndef NETLIST_PATHS_EXCEPTION_HPP
#define NETLIST_PATHS_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace netlist_paths {

/// Base class for netlist_paths exceptions.
struct Exception : public std::exception {
   std::string msg;
   Exception(std::string msg) : msg(msg) {}
   ~Exception() throw () {}
   const char* what() const throw() { return msg.c_str(); }
};

/// An exception from XML elaboration.
struct XMLException : public Exception {
  XMLException(std::string msg) : Exception(msg) {}
};

} // End namespace.

#endif // NETLIST_PATHS_EXCEPTION_HPP

