#ifndef NETLIST_PATHS_READ_VERILATOR_XML_HPP
#define NETLIST_PATHS_READ_VERILATOR_XML_HPP

#include <algorithm>
#include <memory>
#include <stack>
#include <vector>
#include <utility>
#include <boost/algorithm/string/predicate.hpp>
#include <rapidxml-1.13/rapidxml.hpp>

#include "netlist_paths/Debug.hpp"
#include "netlist_paths/Netlist.hpp"

namespace netlist_paths {

using XMLNode = rapidxml::xml_node<>;

class VarNode {
  std::string name;
  VertexDesc vertex;
public:
  VarNode(const std::string &name, VertexDesc vertex) :
      name(name), vertex(vertex) {}
  std::string &getName() { return name; }
  VertexDesc getVertex() { return vertex; }
};

class ScopeNode {
  XMLNode *node;
public:
  ScopeNode(XMLNode *node) : node(node) {}
  XMLNode *getNode() { return node; }
};

class LogicNode {
  XMLNode *node;
  ScopeNode &scope;
  VertexDesc vertex;
public:
  LogicNode(XMLNode *node, ScopeNode &scope, VertexDesc vertex) :
      node(node), scope(scope), vertex(vertex) {}
  XMLNode *getNode() { return node; }
  ScopeNode &getScope() { return scope; }
  VertexDesc getVertex() { return vertex; }
};

class ReadVerilatorXML {
private:
  Netlist &netlist;
  std::vector<File> &files;
  std::vector<DType> &dtypes;
  std::vector<std::unique_ptr<VarNode>> vars;
  std::map<std::string, std::shared_ptr<File>> fileIdMappings;
  std::map<std::string, std::shared_ptr<DType>> dtypeMappings;
  std::stack<std::unique_ptr<LogicNode>> logicParents;
  std::stack<std::unique_ptr<ScopeNode>> scopeParents;
  std::unique_ptr<LogicNode> currentLogic;
  std::unique_ptr<ScopeNode> currentScope;
  bool isDelayedAssign;
  bool isLValue;

  std::shared_ptr<File> addFile(File file) {
    files.push_back(file);
    return std::make_shared<File>(files.back());
  }
  std::shared_ptr<DType> addDtype(DType dtype) {
    dtypes.push_back(dtype);
    return std::make_shared<DType>(dtypes.back());
  }
  std::size_t numChildren(XMLNode *node);
  void dispatchVisitor(XMLNode *node);
  void iterateChildren(XMLNode *node);
  Location parseLocation(const std::string location);
  VertexDesc lookupVarVertex(const std::string &name);
  void newVar(XMLNode *node);
  void newScope(XMLNode *node);
  void newVarScope(XMLNode *node);
  void newStatement(XMLNode *node, VertexAstType);
  void newVarRef(XMLNode *node);
  void visitNode(XMLNode *node);
  void visitModule(XMLNode *node);
  void visitTypeTable(XMLNode *node);
  void visitAlways(XMLNode *node);
  void visitAssign(XMLNode *node);
  void visitAssignDly(XMLNode *node);
  void visitInitial(XMLNode *node);
  void visitSenItem(XMLNode *node);
  void visitSenGate(XMLNode *node);
  void visitCFunc(XMLNode *node);
  void visitScope(XMLNode *node);
  void visitVar(XMLNode *node);
  void visitVarScope(XMLNode *node);
  void visitVarRef(XMLNode *node);
  void visitBasicDtype(XMLNode *node);
  void visitRefDtype(XMLNode *node);
  std::string visitConst(XMLNode *node);
  std::pair<std::string, std::string> visitRange(XMLNode *node);
  void visitArrayDtype(XMLNode *node, bool packed);
  void visitStructDtype(XMLNode *node);
  void readXML(const std::string &filename);

public:
  ReadVerilatorXML() = delete;
  ReadVerilatorXML(Netlist &netlist,
                   std::vector<File> &files,
                   std::vector<DType> &dtypes,
                   const std::string &filename);
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_READ_VERILATOR_XML_HPP
