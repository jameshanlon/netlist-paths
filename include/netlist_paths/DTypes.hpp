#ifndef NETLIST_PATHS_DTYPES_HPP
#define NETLIST_PATHS_DTYPES_HPP

#include <numeric>
#include <memory>
#include <string>
#include <vector>
#include "netlist_paths/Location.hpp"

namespace netlist_paths {

/// Base class for data types.
class DType {

public:
  DType() {}

  /// Return the string representation of the data type.
  ///
  /// \param suffix A suffix to allow unpacked array range specifications to be
  ///               appended with the inner-most dimension on the LHS and
  ///               outermost on the RHS.
  ///
  /// \returns A string representation of the data type.
  virtual const std::string toString(const std::string suffix="") const {
    return name + suffix;
  }

  const std::string getName() const { return name; }
  virtual size_t getWidth() const { return 0; }
  virtual ~DType() = default; // Make DType polymorphic to allow dynamic casts.

protected:
  std::string name;
  Location location;

  DType(Location &location) :
      location(location) {}

  DType(const std::string &name, Location &location) :
      name(name), location(location) {}
};

/// Basic data type with an optional range specified by left and right values.
class BasicDType : public DType {
  unsigned left;
  unsigned right;
  bool ranged;

public:

  /// Construct a basic data type.
  ///
  /// \param name     The name of the type.
  /// \param location The source location of the type.
  BasicDType(const std::string &name, Location &location) :
    DType(name, location), left(0), right(0), ranged(false) {}

  /// Construct a basic data type with a range.
  ///
  /// \param name     The name of the type.
  /// \param location The source location of the type.
  /// \param left     The start index of the range.
  /// \param right    The end index of the range.
  BasicDType(const std::string &name, Location &location,
             unsigned left, unsigned right) :
    DType(name, location), left(left), right(right), ranged(true) {}

  virtual const std::string toString(const std::string suffix="") const override {
    if (ranged) {
      return (boost::format("[%d:%d] %s%s") % left % right % name % suffix).str();
    } else {
      return name+suffix;
    }
  }

  virtual size_t getWidth() const override {
    return ranged ? (left - right + 1) : 1;
  }
};

/// Reference data type, wrapping a sub data type.
class RefDType : public DType {
  std::shared_ptr<DType> subDType;

public:

  /// Construct a reference data type.
  ///
  /// \param name     The name of the type.
  /// \param location The source location of the type.
  RefDType(const std::string &name, Location &location) :
      DType(name, location) {}

  void setSubDType(std::shared_ptr<DType> sdt) { subDType = sdt; }

  virtual const std::string toString(const std::string suffix="") const override {
    return (boost::format("%s%s") % subDType->toString() % suffix).str();
  }

  virtual size_t getWidth() const override {
    return subDType->getWidth();
  }
};

/// Array data type, with a range, packed flag and sub data type.
class ArrayDType : public DType {
  std::shared_ptr<DType> subDType;
  size_t start;
  size_t end;
  bool packed;
public:

  /// Construct an array data type.
  ///
  /// \param location The location of the data type declaration.
  /// \param start    The start of the array index range.
  /// \param end      The end of the array index range.
  /// \param packed   A Boolean to indicate the array is packed.
  ArrayDType(Location &location, size_t start, size_t end, bool packed) :
      DType(location), start(start), end(end), packed(packed) {}

  void setSubDType(std::shared_ptr<DType> sdt) { subDType = sdt; }

  virtual const std::string toString(const std::string suffix="") const override {
    if (packed) {
      // Packed array range specifications are prepended. Eg:
      // [a:b]         [c:d]     <name>
      // ^ innermost ^ outermost
      return (boost::format("[%d:%d] %s%s") % end % start % subDType->toString() % suffix).str();
    } else {
      // Unpacked array range specifications are appended, using the suffix
      // parameter to pack them innermost-to-outermost. Eg:
      // <name> [a:b]         [c:d]
      //        ^ innermost ^ outermost
      auto currentSuffix = (boost::format(" [%d:%d]") % end % start).str();
      return subDType->toString(suffix+currentSuffix);
    }
  }

  virtual size_t getWidth() const override {
    return packed ? (end - start + 1) * subDType->getWidth() : 0;
  }
};

/// Structure or union member data type, wrapping a sub data type.
class MemberDType : public DType {
  std::shared_ptr<DType> subDType;

public:

  /// Construct a member data type.
  ///
  /// \param name     The name of the type.
  /// \param location The source location of the type.
  /// \param sdt      A shared pointer to the sub data type.
  MemberDType(const std::string &name, Location &location,
              std::shared_ptr<DType> sdt) :
      DType(name, location), subDType(sdt) {};

  virtual size_t getWidth() const override {
    return subDType->getWidth();
  }
};

/// Structure data type with a set of members.
class StructDType : public DType {
  std::vector<MemberDType> members;

public:

  /// Construct an unnamed structure data type.
  ///
  /// \param location The source location of the type.
  StructDType(Location &location) :
      DType(location) {}

  /// Construct a named structure data type.
  ///
  /// \param name     The name of the type.
  /// \param location The source location of the type.
  StructDType(const std::string &name, Location &location) :
      DType(name, location) {}

  void addMemberDType(MemberDType memberDType) {
    members.push_back(memberDType);
  }

  virtual const std::string toString(const std::string suffix="") const override {
    return std::string("packed struct") + suffix;
  }

  virtual size_t getWidth() const override {
    auto sum = [](size_t result, const MemberDType &member) { return result + member.getWidth(); };
    return std::accumulate(std::begin(members), std::end(members), 0, sum);
  }
};

/// Union data type with a set of members.
class UnionDType : public DType {
  std::vector<MemberDType> members;

public:

  /// Construct an unnamed union data type.
  ///
  /// \param location The source location of the type.
  UnionDType(Location &location) :
      DType(location) {}

  /// Construct a named union data type.
  ///
  /// \param name     The name of the type.
  /// \param location The source location of the type.
  UnionDType(const std::string &name, Location &location) :
      DType(name, location) {}

  void addMemberDType(MemberDType memberDType) {
    members.push_back(memberDType);
  }

  virtual const std::string toString(const std::string suffix="") const override {
    return std::string("packed union") + suffix;
  }

  virtual size_t getWidth() const override {
    return members.front().getWidth();
  }
};

/// Enumeration item data type, with a name and value.
class EnumItem {
  std::string name;
  size_t value;

public:

  /// Construct an enumeration item data type.
  ///
  /// \param name  The name of the item.
  /// \param value The value of the item.
  EnumItem(const std::string &name, size_t value) :
      name(name), value(value) {}

  const std::string &getName() const { return name; }

  size_t getValue() const { return value; }
};

/// Enumeration data type with a set of items and sub data type.
class EnumDType : public DType {
  std::vector<EnumItem> items;
  std::shared_ptr<DType> subDType;

public:

  /// Construct an enumeration data type.
  ///
  /// \param name     The name of the type.
  /// \param location The source location of the type.
  EnumDType(const std::string &name, Location &location):
      DType(name, location) {}

  void addItem(EnumItem item) { items.push_back(item); }

  void setSubDType(std::shared_ptr<DType> sdt) { subDType = sdt; }

  virtual const std::string toString(const std::string suffix="") const override {
    return std::string("enum") + suffix;
  }

  virtual size_t getWidth() const override {
    return subDType->getWidth();
  }
};

class ClassRefDType : public DType {
public:

  /// Construct a class reference data type.
  ///
  /// \param name     The name of the type.
  /// \param location The source location of the type.
  ClassRefDType(const std::string &name, Location &location) :
      DType(name, location) {}

};

class VoidDType : public DType {
public:

  /// Construct a void data type.
  ///
  /// \param location The source location of the type.
  VoidDType(Location &location):
      DType(std::string(), location) {}
};

} // End namespace.

#endif // NETLIST_PATHS_DTYPES_HPP
