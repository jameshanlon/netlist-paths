#ifndef NETLIST_PATHS_DTYPES_HPP
#define NETLIST_PATHS_DTYPES_HPP

#include <numeric>
#include <memory>
#include <string>
#include <vector>
#include "netlist_paths/Location.hpp"

class DType {
public:
  /// Return the string representation of the dtype. The suffix agrument allows
  /// unpacked array range specifications to be appended with the inner-most
  /// dimension on the LHS and outermost on the RHS.
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

class BasicDType : public DType {
  unsigned left;
  unsigned right;
  bool ranged;
public:
  BasicDType(const std::string &name, Location &location) :
    DType(name, location), left(0), right(0), ranged(false) {}
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

class RefDType : public DType {
  std::shared_ptr<DType> subDType;
public:
  RefDType(const std::string &name, Location &location,
           std::shared_ptr<DType> subDType) :
      DType(name, location), subDType(subDType) {}
  virtual const std::string toString(const std::string suffix="") const override {
    return (boost::format("%s%s") % subDType->toString() % suffix).str();
  }
  virtual size_t getWidth() const override {
    return subDType->getWidth();
  }
};

class ArrayDType : public DType {
  std::shared_ptr<DType> subDType;
  size_t start;
  size_t end;
  bool packed;
public:
  ArrayDType(Location &location, std::shared_ptr<DType> subDType,
             size_t start, size_t end, bool packed) :
      DType(location), subDType(subDType), start(start), end(end), packed(packed) {}
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

class MemberDType : public DType {
  std::shared_ptr<DType> subDType;
public:
  MemberDType(const std::string &name, Location &location,
              std::shared_ptr<DType> subDType) :
      DType(name, location), subDType(subDType) {};
  std::shared_ptr<DType> getSubDType() const { return subDType; }
  virtual size_t getWidth() const override {
    return subDType->getWidth();
  }
};

class StructDType : public DType {
  std::vector<MemberDType> members;
public:
  StructDType(Location &location) :
      DType(location) {}
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

class UnionDType : public DType {
  std::vector<MemberDType> members;
public:
  UnionDType(Location &location) :
      DType(location) {}
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

class EnumItem {
  std::string name;
  size_t value;
public:
  EnumItem(const std::string &name, size_t value) :
      name(name), value(value) {}
  const std::string &getName() const { return name; }
  size_t getValue() const { return value; }
};

class EnumDType : public DType {
  std::vector<EnumItem> items;
  std::shared_ptr<DType> subDType;
public:
  EnumDType(const std::string &name, Location &location,
              std::shared_ptr<DType> subDType) :
      DType(name, location), subDType(subDType) {}
  void addItem(EnumItem item) { items.push_back(item); }
  virtual const std::string toString(const std::string suffix="") const override {
    return std::string("emum") + suffix;
  }
  virtual size_t getWidth() const override {
    return subDType->getWidth();
  }
};

#endif // NETLIST_PATHS_DTYPES_HPP
