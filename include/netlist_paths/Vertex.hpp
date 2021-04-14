#ifndef NETLIST_PATHS_VERTEX_HPP
#define NETLIST_PATHS_VERTEX_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include "netlist_paths/Location.hpp"
#include "netlist_paths/DTypes.hpp"

namespace netlist_paths {

//===----------------------------------------------------------------------===//
// Vertex type enums.
//===----------------------------------------------------------------------===//

/// Vertex types corresponding to the Verilator XML AST format.
enum class VertexAstType {
  LOGIC,
  ASSIGN,
  ASSIGN_ALIAS,
  ASSIGN_DLY,
  ASSIGN_W,
  ALWAYS,
  INITIAL,
  INSTANCE,
  SRC_REG,
  DST_REG,
  SEN_GATE,
  SEN_ITEM,
  VAR,
  WIRE,
  PORT,
  REG_ALIAS,
  C_FUNC,
  INVALID
};

/// Vertex categorisation within the netlist graph, used for selecting
/// collections of vertices with particular properties.
enum class VertexGraphType {
  REG,
  DST_REG,
  SRC_REG,
  REG_ALIAS,
  NET,
  LOGIC,
  PORT,
  START_POINT,
  MID_POINT,
  END_POINT,
  IS_NAMED,
  ANY
};

/// The direction of a variable, applying only to ports.
enum class VertexDirection {
  NONE,
  INPUT,
  OUTPUT,
  INOUT
};

//===----------------------------------------------------------------------===//
// Vertex type helper fuctions.
//===----------------------------------------------------------------------===//

inline VertexAstType getVertexAstType(const std::string &name) {
  static std::map<std::string, VertexAstType> mappings {
      { "LOGIC",        VertexAstType::LOGIC },
      { "ASSIGN",       VertexAstType::ASSIGN },
      { "ASSIGN_ALIAS", VertexAstType::ASSIGN_ALIAS },
      { "ASSIGN_DLY",   VertexAstType::ASSIGN_DLY },
      { "ASSIGN_W",     VertexAstType::ASSIGN_W },
      { "ALWAYS",       VertexAstType::ALWAYS },
      { "INITIAL",      VertexAstType::INITIAL },
      { "INSTANCE",     VertexAstType::INSTANCE },
      { "SRC_REG",      VertexAstType::SRC_REG },
      { "DST_REG",      VertexAstType::DST_REG },
      { "SEN_GATE",     VertexAstType::SEN_GATE },
      { "SEN_ITEM",     VertexAstType::SEN_ITEM },
      { "VAR",          VertexAstType::VAR },
      { "WIRE",         VertexAstType::WIRE },
      { "PORT",         VertexAstType::PORT },
      { "REG_ALIAS",    VertexAstType::REG_ALIAS },
      { "C_FUNC",       VertexAstType::C_FUNC },
  };
  auto it = mappings.find(name);
  return (it != mappings.end()) ? it->second : VertexAstType::INVALID;
}

inline const char *getVertexAstTypeStr(VertexAstType type) {
  switch (type) {
    case VertexAstType::LOGIC:        return "LOGIC";
    case VertexAstType::ASSIGN:       return "ASSIGN";
    case VertexAstType::ASSIGN_ALIAS: return "ASSIGN_ALIAS";
    case VertexAstType::ASSIGN_DLY:   return "ASSIGN_DLY";
    case VertexAstType::ASSIGN_W:     return "ASSIGN_W";
    case VertexAstType::ALWAYS:       return "ALWAYS";
    case VertexAstType::INITIAL:      return "INITIAL";
    case VertexAstType::INSTANCE:     return "INSTANCE";
    case VertexAstType::SRC_REG:      return "SRC_REG";
    case VertexAstType::DST_REG:      return "DST_REG";
    case VertexAstType::SEN_GATE:     return "SEN_GATE";
    case VertexAstType::SEN_ITEM:     return "SEN_ITEM";
    case VertexAstType::VAR:          return "VAR";
    case VertexAstType::WIRE:         return "WIRE";
    case VertexAstType::PORT:         return "PORT";
    case VertexAstType::REG_ALIAS:    return "REG_ALIAS";
    case VertexAstType::C_FUNC:       return "C_FUNC";
    case VertexAstType::INVALID:      return "INVALID";
    default:                          return "UNKNOWN";
  }
}

inline const char *getSimpleVertexAstTypeStr(VertexAstType type) {
  switch (type) {
    case VertexAstType::LOGIC:        return "LOGIC";
    case VertexAstType::ASSIGN:       return "ASSIGN";
    case VertexAstType::ASSIGN_ALIAS: return "ASSIGN";
    case VertexAstType::ASSIGN_DLY:   return "ASSIGN";
    case VertexAstType::ASSIGN_W:     return "ASSIGN";
    case VertexAstType::ALWAYS:       return "ALWAYS";
    case VertexAstType::INITIAL:      return "INITIAL";
    case VertexAstType::INSTANCE:     return "INSTANCE";
    case VertexAstType::SRC_REG:      return "REG";
    case VertexAstType::DST_REG:      return "REG";
    case VertexAstType::SEN_GATE:     return "SEN";
    case VertexAstType::SEN_ITEM:     return "SEN";
    case VertexAstType::VAR:          return "VAR";
    case VertexAstType::WIRE:         return "WIRE";
    case VertexAstType::PORT:         return "PORT";
    case VertexAstType::REG_ALIAS:    return "VAR";
    case VertexAstType::C_FUNC:       return "C_FUNCTION";
    case VertexAstType::INVALID:      return "INVALID";
    default:                          return "UNKNOWN";
  }
}

inline VertexDirection getVertexDirection(const std::string &direction) {
  static std::map<std::string, VertexDirection> mappings {
      { "input",  VertexDirection::INPUT },
      { "output", VertexDirection::OUTPUT },
      { "inout",  VertexDirection::INOUT },
  };
  auto it = mappings.find(direction);
  return (it != mappings.end()) ? it->second : VertexDirection::NONE;
}

inline const char *getVertexDirectionStr(VertexDirection direction) {
  switch (direction) {
    case VertexDirection::NONE:   return "NONE";
    case VertexDirection::INPUT:  return "INPUT";
    case VertexDirection::OUTPUT: return "OUTPUT";
    case VertexDirection::INOUT:  return "INOUT";
    default:                      return "UNKNOWN";
  }
}

//===----------------------------------------------------------------------===//
// Vertex
//===----------------------------------------------------------------------===//

/// A class representing a vertex in the netlist graph.
class Vertex {
  VertexAstType astType;
  VertexDirection direction;
  Location location;
  std::shared_ptr<DType> dtype;
  std::string name;
  bool isParam;
  std::string paramValue;
  bool publicVisibility;
  bool top;
  bool deleted;

public:
  Vertex() {}

  /// Construct a logic vertex.
  ///
  /// \param type     The AST type of the logic statement.
  /// \param location The source location of the logic statement.
  Vertex(VertexAstType type,
         Location location) :
      astType(type),
      direction(VertexDirection::NONE),
      location(location),
      isParam(false),
      publicVisibility(false),
      top(false),
      deleted(false) {}

  /// Construct a variable vertex.
  ///
  /// \param type             The AST type of the variable.
  /// \param direction        The direction of the variable type.
  /// \param location         The source location of the variable declaration.
  /// \param dtype            The data type of the variable.
  /// \param name             The name of the variable.
  /// \param isParam          A flag indicating the variable is a parameter.
  /// \param paramValue       The value of the parameter variable.
  /// \param publicVisibility A flag indicating the variable has public visibility.
  Vertex(VertexAstType type,
         VertexDirection direction,
         Location location,
         std::shared_ptr<DType> dtype,
         const std::string &name,
         bool isParam,
         const std::string &paramValue,
         bool publicVisibility) :
      astType(type),
      direction(direction),
      location(location),
      dtype(dtype),
      name(name),
      isParam(isParam),
      paramValue(paramValue),
      publicVisibility(publicVisibility),
      top(determineIsTop(name)),
      deleted(false) {}

  /// Copy constructor.
  Vertex(const Vertex &v) :
      astType(v.astType),
      direction(v.direction),
      location(v.location),
      dtype(v.dtype),
      name(v.name),
      isParam(v.isParam),
      top(v.top),
      deleted(v.deleted) {}

  /// Return whether a variable name is in the top scope.
  ///
  /// A variable is in the 'top' scope when has one or two hierarchical
  /// components. For example, module.name or name is top level, but
  /// module.submodule.name is not.
  ///
  /// \param name The name of a variable.
  ///
  /// \returns Whether the variable is in the top scope.
  static bool determineIsTop(const std::string &name) {
    std::vector<std::string> tokens;
    boost::split(tokens, name, boost::is_any_of("."));
    return tokens.size() < 3;
  }

  /// Given a hierarchical variable name, eg a.b.c, return the last component c.
  ///
  /// \returns The last heirarchical component of a variable name.
  std::string getBasename() const {
    std::vector<std::string> tokens;
    boost::split(tokens, name, boost::is_any_of("."));
    return tokens.back();
  }

  /// Match this vertex against different graph types.
  ///
  /// \param type A categorisation of a vertex.
  ///
  /// \returns Whether the vertex matches the specified type.
  bool isGraphType(VertexGraphType type) const {
    switch (type) {
      case VertexGraphType::REG:         return isReg();
      case VertexGraphType::REG_ALIAS:   return isRegAlias();
      case VertexGraphType::SRC_REG:     return isSrcReg();
      case VertexGraphType::DST_REG:     return isDstReg();
      case VertexGraphType::LOGIC:       return isLogic();
      case VertexGraphType::NET:         return isNet();
      case VertexGraphType::PORT:        return isPort();
      case VertexGraphType::START_POINT: return isStartPoint();
      case VertexGraphType::END_POINT:   return isEndPoint();
      case VertexGraphType::MID_POINT:   return isMidPoint();
      case VertexGraphType::IS_NAMED:    return isNamed();
      default:                           return false;
    }
  }

  /// Less than comparison between two Vertex objects.
  bool compareLessThan(const Vertex &b) const {
    if (name        < b.name)      return true;
    if (b.name      < name)        return false;
    if (astType     < b.astType)   return true;
    if (b.astType   < astType)     return false;
    if (direction   < b.direction) return true;
    if (b.direction < direction)   return false;
    if (deleted     < b.deleted)   return true;
    if (b.deleted   < deleted)     return false;
    return false;
  }

  /// Equality comparison between two vertex objects.
  bool compareEqual(const Vertex &b) const {
    return astType          == b.astType &&
           direction        == b.direction &&
           location         == b.location &&
           dtype            == b.dtype &&
           name             == b.name &&
           isParam          == b.isParam &&
           paramValue       == b.paramValue &&
           publicVisibility == b.publicVisibility &&
           top              == b.top &&
           deleted          == b.deleted;
  }

  /// Return true if the vertex is in the top scope.
  inline bool isTop() const { return top; }

  /// Return true if the vertex has public visibility.
  inline bool isPublic() const { return publicVisibility; }

  /// Return true if the vertex is a source register.
  inline bool isSrcReg() const {
    return !deleted && astType == VertexAstType::SRC_REG;
  }

  /// Return true if the vertex is a destination register.
  inline bool isDstReg() const {
    return !deleted && astType == VertexAstType::DST_REG;
  }

  /// Return true if the vertex is a logic statement.
  inline bool isLogic() const {
    return astType == VertexAstType::LOGIC ||
           astType == VertexAstType::ASSIGN ||
           astType == VertexAstType::ASSIGN_ALIAS ||
           astType == VertexAstType::ASSIGN_DLY ||
           astType == VertexAstType::ASSIGN_W ||
           astType == VertexAstType::ALWAYS ||
           astType == VertexAstType::INITIAL ||
           astType == VertexAstType::SEN_GATE ||
           astType == VertexAstType::SEN_ITEM;
  }

  /// Return true if the vertex is a parameter variable.
  inline bool isParameter() const {
    return isParam;
  }

  /// Return true if the vertex is a net variable.
  inline bool isNet() const {
    return !deleted && !isPort() && !isReg() && !isLogic() && !isParameter();
  }

  /// Return true if the vertex is a register variable.
  inline bool isReg() const {
    return !deleted &&
           (astType == VertexAstType::SRC_REG ||
            astType == VertexAstType::DST_REG);
  }

  /// Return true if the vertex is an alias of a register variable.
  inline bool isRegAlias() const {
    return !deleted &&
           astType == VertexAstType::REG_ALIAS;
  }

  /// Return true if the vertex is a port variable.
  inline bool isPort() const {
    return !deleted &&
           top &&
           (direction == VertexDirection::INPUT ||
            direction == VertexDirection::OUTPUT ||
            direction == VertexDirection::INOUT);
  }

  /// Return true if the vertex is a valid start point for a path.
  inline bool isStartPoint() const {
    return !deleted &&
           (astType == VertexAstType::SRC_REG ||
            (direction == VertexDirection::INPUT && top) ||
            (direction == VertexDirection::INOUT && top));
  }

  /// Return true if the vertex is a valid end point for a path.
  inline bool isEndPoint() const {
    return !deleted &&
           (astType == VertexAstType::DST_REG ||
            (direction == VertexDirection::OUTPUT && top) ||
            (direction == VertexDirection::INOUT && top));
  }

  /// Return true if the vertex is a valid mid point for a path.
  inline bool isMidPoint() const {
    return !isStartPoint() && !isEndPoint();
  }

  /// Return true if the vertex has been introduced by Verilator.
  inline bool canIgnore() const {
    if (!name.empty()) {
      return name.find("__Vdly") != std::string::npos ||
             name.find("__Vcell") != std::string::npos ||
             name.find("__Vconc") != std::string::npos ||
             name.find("__Vfunc") != std::string::npos;
    } else {
      return false;
    }
  }

  /// Return true if the vertex has a name that should be included in reports.
  inline bool isNamed() const {
    return !isLogic() &&
           !isSrcReg() &&
           !canIgnore() &&
           !isDeleted();
  }

  /// Return a string description of this vertex.
  std::string toString() const {
    // TODO: expand on this for different vertex types.
    return std::string(getVertexAstTypeStr(astType));
  }

  /// Various getters and setters.
  void setDeleted() { deleted = true; }
  void setVar() { astType = VertexAstType::VAR; }
  void setSrcReg() { astType = VertexAstType::SRC_REG; }
  void setDstReg() { astType = VertexAstType::DST_REG; }
  void setRegAlias() { astType = VertexAstType::REG_ALIAS; }
  void setDirection(VertexDirection dir) { direction = dir; }
  VertexAstType getAstType() const { return astType; }
  VertexDirection getDirection() const { return direction; }
  size_t getDTypeWidth() const { return dtype != nullptr ? dtype->getWidth() : 0; }
  DType *getDTypePtr() const {
    // Remove the const cast to make it compatible with the boost::python wrappers.
    return const_cast<DType*>(dtype.get());
  }
  const std::string getName() const { return name; }
  const std::string getAstTypeStr() const { return getVertexAstTypeStr(astType); }
  const std::string getSimpleAstTypeStr() const { return getSimpleVertexAstTypeStr(astType); }
  const std::string getDirStr() const { return getVertexDirectionStr(direction); }
  const std::string getDTypeStr() const { return dtype != nullptr ? dtype->toString() : "-"; }
  const std::string getLocationStr() const { return location.getLocationStr(); }
  bool isDeleted() const { return deleted; }
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_VERTEX_HPP
