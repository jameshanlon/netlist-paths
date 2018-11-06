#ifndef NETLIST_PATHS_GRAPH_HPP
#define NETLIST_PATHS_GRAPH_HPP

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace netlist_paths {

enum class VertexType {
  NONE,
  LOGIC,
  ASSIGN,
  ASSIGNW,
  ALWAYS,
  INITIAL,
  REG_SRC,
  REG_DST,
  VAR,
  WIRE,
  PORT
};

enum class VertexDirection {
  NONE,
  INPUT,
  OUTPUT,
  INOUT
};

inline bool isVertexTypeLogic(VertexType type) {
  return type == VertexType::LOGIC ||
         type == VertexType::ASSIGN ||
         type == VertexType::ASSIGNW ||
         type == VertexType::ALWAYS ||
         type == VertexType::INITIAL;
}

struct Vertex {
  int id;
  VertexType type;
  VertexDirection direction;
  std::string name;
  std::string loc;
  bool isTop;
  // Logic vertex.
  Vertex(int id,
         VertexType type,
         const std::string &loc) :
    id(id), type(type), direction(VertexDirection::NONE), loc(loc) {}
  // Variable vertex.
  Vertex(int id,
         VertexType type,
         VertexDirection direction,
         const std::string &name,
         const std::string &loc) :
      id(id), type(type), name(name), loc(loc) {
    // Check there are no Vlvbound nodes.
    assert(name.find("__Vlvbound") == std::string::npos);
    // module.name or name is top level, but module.submodule.name is not.
    std::vector<std::string> tokens;
    boost::split(tokens, name, boost::is_any_of("."));
    isTop = tokens.size() < 3;
  }
  bool isLogic() const {
    return isVertexTypeLogic(type);
  }
  bool isStartPoint() const {
    return type == VertexType::REG_SRC ||
           (direction == VertexDirection::INPUT && isTop) ||
           (direction == VertexDirection::INOUT && isTop);
  }
  bool isEndPoint() const {
    return type == VertexType::REG_DST ||
           (direction == VertexDirection::OUTPUT && isTop) ||
           (direction == VertexDirection::INOUT && isTop);
  }
  bool isReg() const {
    return type == VertexType::REG_DST ||
           type == VertexType::REG_SRC;
  }
  bool canIgnore() const {
    // Ignore variables Verilator has introduced.
    return name.find("__Vdly") != std::string::npos ||
           name.find("__Vcell") != std::string::npos;
  }
};

struct Edge {
  int src;
  int dst;
  Edge(int src, int dst) : src(src), dst(dst) {}
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_GRAPH_HPP
