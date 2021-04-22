#ifndef NETLIST_PATHS_EXCEPTION_HPP
#define NETLIST_PATHS_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace netlist_paths {

/// Base class for netlist_paths exceptions.
class Exception : public std::exception {
   std::string msg;

public:
  /// Construct a new exception.
  ///
  /// \param msg The exception message.
  Exception(std::string msg) : msg(msg) {}

  ~Exception() throw () {}

  /// Return the exception message.
  const char* what() const throw() { return msg.c_str(); }
};

/// An exception from XML elaboration.
class XMLException : public Exception {

public:

  /// Construct a new XML exception.
  ///
  /// \param msg The exception message.
  XMLException(std::string msg) : Exception(msg) {}
};

} // End namespace.

#endif // NETLIST_PATHS_EXCEPTION_HPP

