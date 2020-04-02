#ifndef NETLIST_PATHS_GRAPH_HPP
#define NETLIST_PATHS_GRAPH_HPP

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "netlist_paths/Exception.hpp"

namespace netlist_paths {

enum VertexType {
  LOGIC,
  ASSIGN,
  ASSIGNW,
  ALWAYS,
  INITIAL,
  REG_SRC,
  REG_DST,
  VAR,
  WIRE,
  PORT,
  CFUNC
};

enum VertexDirection {
  NONE,
  INPUT,
  OUTPUT,
  INOUT
};

struct VertexProperties {
  unsigned long long id;
  VertexType type;
  VertexDirection dir;
  unsigned long width;
  std::string name;
  std::string loc;
  bool isTop;
  bool deleted;
};

// Vertex helper fuctions.

inline bool determineIsTop(const std::string &name) {
  // Check there are no Vlvbound nodes.
  assert(name.find("__Vlvbound") == std::string::npos);
  // module.name or name is top level, but module.submodule.name is not.
  std::vector<std::string> tokens;
  boost::split(tokens, name, boost::is_any_of("."));
  return tokens.size() < 3;
}

inline std::string expandName(const std::string &topName,
                              const std::string &name) {
  // Add prefix to top-level names that are missing it.
  if (name.rfind(topName + ".", 0) == std::string::npos) {
    return std::string() + topName + "." + name;
  }
  return name;
}

inline bool isSrcReg(const VertexProperties &p) {
  return !p.deleted &&
         p.type == VertexType::REG_SRC;
}

inline bool isLogic(const VertexProperties &p) {
  return p.type == VertexType::LOGIC ||
         p.type == VertexType::ASSIGN ||
         p.type == VertexType::ASSIGNW ||
         p.type == VertexType::ALWAYS ||
         p.type == VertexType::INITIAL;
}

inline bool isReg(const VertexProperties &p) {
  return !p.deleted &&
         (p.type == VertexType::REG_DST ||
          p.type == VertexType::REG_SRC);
}

inline bool isStartPoint(const VertexProperties &p) {
  return !p.deleted &&
         (p.type == VertexType::REG_SRC ||
          (p.dir == VertexDirection::INPUT && p.isTop) ||
          (p.dir == VertexDirection::INOUT && p.isTop));
}

inline bool isEndPoint(const VertexProperties &p) {
  return !p.deleted &&
         (p.type == VertexType::REG_DST ||
          (p.dir == VertexDirection::OUTPUT && p.isTop) ||
          (p.dir == VertexDirection::INOUT && p.isTop));
}

inline bool isMidPoint(const VertexProperties &p) {
  return !p.deleted &&
         (p.type == VertexType::VAR ||
          p.type == VertexType::WIRE ||
          p.type == VertexType::PORT);
}

inline bool canIgnore(const VertexProperties &p) {
  // Ignore variables Verilator has introduced.
  return p.name.find("__Vdly") != std::string::npos ||
         p.name.find("__Vcell") != std::string::npos ||
         p.name.find("__Vconc") != std::string::npos;
}

inline VertexType getVertexType(const std::string &type) {
       if (type == "LOGIC")   return VertexType::LOGIC;
  else if (type == "ASSIGN")  return VertexType::ASSIGN;
  else if (type == "ASSIGNW") return VertexType::ASSIGNW;
  else if (type == "ALWAYS")  return VertexType::ALWAYS;
  else if (type == "INITIAL") return VertexType::INITIAL;
  else if (type == "REG_SRC") return VertexType::REG_SRC;
  else if (type == "REG_DST") return VertexType::REG_DST;
  else if (type == "VAR")     return VertexType::VAR;
  else if (type == "WIRE")    return VertexType::WIRE;
  else if (type == "PORT")    return VertexType::PORT;
  else if (type == "CFUNC")   return VertexType::CFUNC;
  else {
    throw Exception(std::string("unexpected vertex type: ")+type);
  }
}

inline const char *getVertexTypeStr(VertexType type) {
  switch (type) {
    case VertexType::LOGIC:   return "LOGIC";
    case VertexType::ASSIGN:  return "ASSIGN";
    case VertexType::ASSIGNW: return "ASSIGNW";
    case VertexType::ALWAYS:  return "ALWAYS";
    case VertexType::INITIAL: return "INITIAL";
    case VertexType::REG_SRC: return "REG_SRC";
    case VertexType::REG_DST: return "REG_DST";
    case VertexType::VAR:     return "VAR";
    case VertexType::WIRE:    return "WIRE";
    case VertexType::PORT:    return "PORT";
    case VertexType::CFUNC:   return "CFUNC";
    default:                  return "UNKNOWN";
  }
}

inline VertexDirection getVertexDirection(const std::string &direction) {
       if (direction == "NONE")   return VertexDirection::NONE;
  else if (direction == "INPUT")  return VertexDirection::INPUT;
  else if (direction == "OUTPUT") return VertexDirection::OUTPUT;
  else if (direction == "INOUT")  return VertexDirection::INOUT;
  else {
    throw Exception(std::string("unexpected vertex direction: ")+direction);
  }
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

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_GRAPH_HPP
