#ifndef NETLIST_PATHS_EXCEPTION_HPP
#define NETLIST_PATHS_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace netlist_paths {

struct Exception : public std::exception {
   std::string msg;
   Exception(std::string msg) : msg(msg) {}
   ~Exception() throw () {}
   const char* what() const throw() { return msg.c_str(); }
};

struct XMLException : public Exception {
  XMLException(std::string msg) : Exception(msg) {}
};

} // End namespace.

#endif // NETLIST_PATHS_EXCEPTION_HPP

