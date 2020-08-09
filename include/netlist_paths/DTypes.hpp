#ifndef NETLIST_PATHS_DTYPES_HPP
#define NETLIST_PATHS_DTYPES_HPP

#include <string>
#include <vector>
#include "netlist_paths/Location.hpp"

struct DType {
  std::string name;
  Location location;
protected:
  DType(const std::string &name, Location &location) :
      name(name), location(location) {}
};

class BasicDType : public DType {
  unsigned left;
  unsigned right;
public:
    BasicDType(const std::string &name, Location &location) :
        DType(name, location), left(0), right(0) {}
    BasicDType(const std::string &name, Location &location,
               unsigned left, unsigned right) :
        DType(name, location), left(left), right(right) {}
};

class RefDType : public DType {
public:
  RefDType(const std::string &name, Location &location) :
      DType(name, location) {}
};

class ArrayDType : public DType {
  std::string start;
  std::string end;
  bool packed;
public:
  ArrayDType(const std::string &name, Location &location,
             std::string &start, std::string &end, bool packed) :
      DType(name, location), start(start), end(end), packed(packed) {}
};

class StructDType : public DType {
public:
    StructDType(const std::string &name, Location &location) :
        DType(name, location){}
};

#endif // NETLIST_PATHS_DTYPES_HPP
