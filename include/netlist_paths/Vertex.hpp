#ifndef NETLIST_PATHS_VERTEX_HPP
#define NETLIST_PATHS_VERTEX_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include <boost/graph/graph_traits.hpp>
#include "netlist_paths/Location.hpp"
#include "netlist_paths/DTypes.hpp"
#include "netlist_paths/Options.hpp"

namespace netlist_paths {

//===----------------------------------------------------------------------===//
// Vertex type enums.
//===----------------------------------------------------------------------===//

/// Vertex types corresponding to the Verilator XML AST format.
enum class VertexAstType {
  ALWAYS,
  ASSIGN,
  ASSIGN_ALIAS,
  ASSIGN_DLY,
  ASSIGN_W,
  CASE,
  C_CALL,
  C_FUNC,
  C_METHOD_CALL,
  C_STMT,
  DISPLAY,
  DST_REG,
  DST_REG_ALIAS,
  FINISH,
  IF,
  INITIAL,
  INSTANCE,
  JUMP_BLOCK,
  LOGIC,
  PORT,
  READ_MEM,
  SEN_GATE,
  SFORMATF,
  SRC_REG,
  SRC_REG_ALIAS,
  VAR,
  WHILE,
  WIRE,
  INVALID
};

/// Vertex categorisation within the netlist graph, used for selecting
/// collections of vertices with particular properties.
enum class VertexNetlistType {
  DST_REG,
  DST_REG_ALIAS,
  END_POINT,
  IS_NAMED,
  LOGIC,
  MID_POINT,
  NET,
  PORT,
  REG,
  SRC_REG,
  SRC_REG_ALIAS,
  START_POINT,
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
      { "ALWAYS",        VertexAstType::ALWAYS },
      { "ASSIGN",        VertexAstType::ASSIGN },
      { "ASSIGN_ALIAS",  VertexAstType::ASSIGN_ALIAS },
      { "ASSIGN_DLY",    VertexAstType::ASSIGN_DLY },
      { "ASSIGN_W",      VertexAstType::ASSIGN_W },
      { "CASE",          VertexAstType::CASE },
      { "C_FUNC",        VertexAstType::C_FUNC },
      { "C_METHOD_CALL", VertexAstType::C_METHOD_CALL },
      { "C_STMT",        VertexAstType::C_STMT },
      { "DISPLAY",       VertexAstType::DISPLAY },
      { "DST_REG",       VertexAstType::DST_REG },
      { "DST_REG_ALIAS", VertexAstType::DST_REG_ALIAS },
      { "FINISH",        VertexAstType::FINISH },
      { "IF",            VertexAstType::IF },
      { "INITIAL",       VertexAstType::INITIAL },
      { "INSTANCE",      VertexAstType::INSTANCE },
      { "JUMP_BLOCK",    VertexAstType::JUMP_BLOCK },
      { "LOGIC",         VertexAstType::LOGIC },
      { "PORT",          VertexAstType::PORT },
      { "READ_MEM",      VertexAstType::READ_MEM },
      { "SEN_GATE",      VertexAstType::SEN_GATE },
      { "SFORMATF",      VertexAstType::SFORMATF },
      { "SRC_REG",       VertexAstType::SRC_REG },
      { "SRC_REG_ALIAS", VertexAstType::SRC_REG_ALIAS },
      { "VAR",           VertexAstType::VAR },
      { "WHILE",         VertexAstType::WHILE },
      { "WIRE",          VertexAstType::WIRE },
  };
  auto it = mappings.find(name);
  return (it != mappings.end()) ? it->second : VertexAstType::INVALID;
}

/// Return string representations of the VertexAstType enum.
inline const char *getVertexAstTypeStr(VertexAstType type) {
  switch (type) {
    case VertexAstType::ALWAYS:        return "ALWAYS";
    case VertexAstType::ASSIGN:        return "ASSIGN";
    case VertexAstType::ASSIGN_ALIAS:  return "ASSIGN_ALIAS";
    case VertexAstType::ASSIGN_DLY:    return "ASSIGN_DLY";
    case VertexAstType::ASSIGN_W:      return "ASSIGN_W";
    case VertexAstType::CASE:          return "CASE";
    case VertexAstType::C_FUNC:        return "C_FUNC";
    case VertexAstType::C_METHOD_CALL: return "C_METHOD_CALL";
    case VertexAstType::C_STMT:        return "C_STMT";
    case VertexAstType::DISPLAY:       return "DISPLAY";
    case VertexAstType::DST_REG:       return "DST_REG";
    case VertexAstType::DST_REG_ALIAS: return "DST_REG_ALIAS";
    case VertexAstType::FINISH:        return "FINISH";
    case VertexAstType::IF:            return "IF";
    case VertexAstType::INITIAL:       return "INITIAL";
    case VertexAstType::INSTANCE:      return "INSTANCE";
    case VertexAstType::INVALID:       return "INVALID";
    case VertexAstType::LOGIC:         return "LOGIC";
    case VertexAstType::PORT:          return "PORT";
    case VertexAstType::READ_MEM:      return "READ_MEM";
    case VertexAstType::SEN_GATE:      return "SEN_GATE";
    case VertexAstType::SFORMATF:      return "SFORMATF";
    case VertexAstType::SRC_REG:       return "SRC_REG";
    case VertexAstType::SRC_REG_ALIAS: return "SRC_REG_ALIAS";
    case VertexAstType::VAR:           return "VAR";
    case VertexAstType::WHILE:         return "WHILE";
    case VertexAstType::WIRE:          return "WIRE";
    default:                           return "UNKNOWN";
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
  size_t id;
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
      id(v.id),
      astType(v.astType),
      direction(v.direction),
      location(v.location),
      dtype(v.dtype),
      name(v.name),
      isParam(v.isParam),
      top(v.top),
      deleted(v.deleted) {}

  /// Return whether a variable name is a top signal.
  ///
  /// A variable is 'top' when it is not prefixed with any hierarchy path.
  /// This applies to top-level ports and parameters only, since all other
  /// variables exist within a module scope.
  ///
  /// \param name The name of a variable.
  ///
  /// \returns Whether the variable is in the top scope.
  static bool determineIsTop(const std::string &name) {
    std::vector<std::string> tokens;
    boost::split(tokens, name, boost::is_any_of("."));
    return tokens.size() == 1;
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
  bool isGraphType(VertexNetlistType type) const {
    switch (type) {
      case VertexNetlistType::REG:           return isReg();
      case VertexNetlistType::SRC_REG_ALIAS: return isSrcRegAlias();
      case VertexNetlistType::DST_REG_ALIAS: return isDstRegAlias();
      case VertexNetlistType::SRC_REG:       return isSrcReg();
      case VertexNetlistType::DST_REG:       return isDstReg();
      case VertexNetlistType::LOGIC:         return isLogic();
      case VertexNetlistType::NET:           return isNet();
      case VertexNetlistType::PORT:          return isPort();
      case VertexNetlistType::START_POINT:   return isStartPoint();
      case VertexNetlistType::END_POINT:     return isEndPoint();
      case VertexNetlistType::MID_POINT:     return isMidPoint();
      case VertexNetlistType::IS_NAMED:      return isNamed();
      default:                               return false;
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
           astType == VertexAstType::ALWAYS ||
           astType == VertexAstType::ASSIGN ||
           astType == VertexAstType::ASSIGN_ALIAS ||
           astType == VertexAstType::ASSIGN_DLY ||
           astType == VertexAstType::ASSIGN_W ||
           astType == VertexAstType::C_CALL ||
           astType == VertexAstType::C_FUNC ||
           astType == VertexAstType::C_METHOD_CALL ||
           astType == VertexAstType::C_STMT ||
           astType == VertexAstType::DISPLAY ||
           astType == VertexAstType::FINISH ||
           astType == VertexAstType::IF ||
           astType == VertexAstType::INITIAL ||
           astType == VertexAstType::SEN_GATE ||
           astType == VertexAstType::SFORMATF ||
           astType == VertexAstType::WHILE;
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

  /// Return true if the vertex is an alias of a source register variable.
  inline bool isSrcRegAlias() const {
    return !deleted && astType == VertexAstType::SRC_REG_ALIAS;
  }

  /// Return true if the vertex is an alias of a destination register variable.
  inline bool isDstRegAlias() const {
    return !deleted && astType == VertexAstType::DST_REG_ALIAS;
  }

  /// Return true if the vertex is a port variable.
  /// Handle a special case where port registers have REG VertexAstType.
  inline bool isPort() const {
    auto isAstPort = astType == VertexAstType::PORT;
    auto isRegPort = top && isReg() && (direction == VertexDirection::OUTPUT);
    return !deleted && (isAstPort || isRegPort);
  }

  /// Return true if the vertex is a valid start point for a combinatorial path
  /// within the netlist.
  ///   - Source register
  ///   - Alias of a source register
  ///   - A top input or inout port
  inline bool isCombStartPoint() const {
    return !deleted &&
           (astType == VertexAstType::SRC_REG ||
            astType == VertexAstType::SRC_REG_ALIAS ||
            (direction == VertexDirection::INPUT && top) ||
            (direction == VertexDirection::INOUT && top));
  }

  /// Return true if the vertex is a valid end point for a combinatorial path
  /// within the netlist.
  ///   - Destination register
  ///   - Alias of a destination register
  ///   - A top output or inout port
  inline bool isCombEndPoint() const {
    return !deleted &&
           (astType == VertexAstType::DST_REG ||
            astType == VertexAstType::DST_REG_ALIAS ||
            (direction == VertexDirection::OUTPUT && top) ||
            (direction == VertexDirection::INOUT && top));
  }

  /// Return true if the vertex is a valid start point for a path.
  inline bool isStartPoint() const {
    if (Options::getInstance().isRestrictStartPoints()) {
      return isCombStartPoint();
    } else {
      // Otherwise, a start point can be any non-destination vertex that is not
      // ignored or deleted.
      return !isDstReg() && !isDstRegAlias() && !canIgnore() && !deleted;
    }
  }

  /// Return true if the vertex is a valid end point for a path.
  inline bool isEndPoint() const {
    if (Options::getInstance().isRestrictEndPoints()) {
      return isCombEndPoint();
    } else {
      // Otherwise, an end point can be any non-destination vertex that is not
      // ignored or deleted.
      return !isSrcReg() && !isSrcRegAlias() && !canIgnore() && !deleted;
    }
  }

  /// Return true if the vertex is a valid mid point for a path.
  inline bool isMidPoint() const {
    if (Options::getInstance().shouldTraverseRegisters()) {
      // Any node is a valid mid point if registers are traversed.
      return isNamed();
    } else {
      // Otherwise, a mid point is otherwise any node that does not start or end
      // a combinatorial path.
      return !isCombStartPoint() && !isCombEndPoint() && !canIgnore() && !deleted;
    }
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

  /// Return true if the vertex has a name, ie is a variable of some description.
  inline bool isNamed() const {
    return !isLogic() &&
           !isSrcReg() &&
           !isSrcRegAlias() &&
           !canIgnore() &&
           !deleted;
  }

  /// Map Vertex AST types to useful names that can be included in reports.
  const char *getSimpleAstTypeStr() const {
    switch (astType) {
      case VertexAstType::ALWAYS:        return "ALWAYS";
      case VertexAstType::ASSIGN:        return "ASSIGN";
      case VertexAstType::ASSIGN_ALIAS:  return "ASSIGN";
      case VertexAstType::ASSIGN_DLY:    return "ASSIGN";
      case VertexAstType::ASSIGN_W:      return "ASSIGN";
      case VertexAstType::CASE:          return "CASE";
      case VertexAstType::C_FUNC:        return "C_FUNCTION";
      case VertexAstType::C_METHOD_CALL: return "C_METHOD_CALL";
      case VertexAstType::C_STMT:        return "C_STATEMENT";
      case VertexAstType::DISPLAY:       return "DISPLAY";
      case VertexAstType::DST_REG:       return isPort() ? "PORT_REG" : "REG";
      case VertexAstType::DST_REG_ALIAS: return "REG_ALIAS";
      case VertexAstType::FINISH:        return "FINISH";
      case VertexAstType::IF:            return "IF";
      case VertexAstType::INITIAL:       return "INITIAL";
      case VertexAstType::INSTANCE:      return "INSTANCE";
      case VertexAstType::INVALID:       return "INVALID";
      case VertexAstType::LOGIC:         return "LOGIC";
      case VertexAstType::PORT:          return "PORT";
      case VertexAstType::READ_MEM:      return "READ_MEM";
      case VertexAstType::SEN_GATE:      return "SEN";
      case VertexAstType::SFORMATF:      return "SFORMATF";
      case VertexAstType::SRC_REG:       return isPort() ? "PORT_REG" : "REG";
      case VertexAstType::SRC_REG_ALIAS: return "REG_ALIAS";
      case VertexAstType::VAR:           return isPort() ? "PORT_VAR" : "VAR";
      case VertexAstType::WHILE:         return "WHILE";
      case VertexAstType::WIRE:          return "WIRE";
      default:                           return "UNKNOWN";
    }
  }

  /// Return a string description of this vertex.
  std::string toString() const {
    // TODO: expand on this for different vertex types.
    return std::string(getVertexAstTypeStr(astType));
  }

  //===--------------------------------------------------------------------===//
  // Various getters and setters.
  //===--------------------------------------------------------------------===//

  void setID(size_t idValue) { id = idValue; }
  void setDeleted() { deleted = true; }
  void setVar() { astType = VertexAstType::VAR; }
  void setSrcReg() { astType = VertexAstType::SRC_REG; }
  void setDstReg() { astType = VertexAstType::DST_REG; }
  void setSrcRegAlias() { astType = VertexAstType::SRC_REG_ALIAS; }
  void setDstRegAlias() { astType = VertexAstType::DST_REG_ALIAS; }
  void setDirection(VertexDirection dir) { direction = dir; }

  VertexAstType getAstType() const { return astType; }
  VertexDirection getDirection() const { return direction; }
  size_t getDTypeWidth() const { return dtype != nullptr ? dtype->getWidth() : 0; }
  DType *getDTypePtr() const {
    // Remove the const cast to make it compatible with the boost::python wrappers.
    return const_cast<DType*>(dtype.get());
  }
  size_t getID() const { return id; }
  const std::string getName() const { return name; }
  const std::string getAstTypeStr() const { return getVertexAstTypeStr(astType); }
  const std::string getDirStr() const { return getVertexDirectionStr(direction); }
  const std::string getDTypeStr() const { return dtype != nullptr ? dtype->toString() : "-"; }
  const std::string getLocationStr() const { return location.getLocationStr(); }
  bool isDeleted() const { return deleted; }
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_VERTEX_HPP
