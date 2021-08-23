#ifndef NETLIST_PATHS_READ_VERILATOR_XML_HPP
#define NETLIST_PATHS_READ_VERILATOR_XML_HPP

#include <algorithm>
#include <memory>
#include <stack>
#include <vector>
#include <utility>
#include <boost/algorithm/string/predicate.hpp>
#include <rapidxml-1.13/rapidxml.hpp>

#include "netlist_paths/Graph.hpp"

namespace netlist_paths {

using XMLNode = rapidxml::xml_node<>;

class VarNode {
  std::string name;
  VertexID vertex;
public:
  VarNode(const std::string &name, VertexID vertex) :
      name(name), vertex(vertex) {}
  std::string &getName() { return name; }
  VertexID getVertex() { return vertex; }
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
  VertexID vertex;
public:
  LogicNode(XMLNode *node, ScopeNode &scope, VertexID vertex) :
      node(node), scope(scope), vertex(vertex) {}
  XMLNode *getNode() { return node; }
  ScopeNode &getScope() { return scope; }
  VertexID getVertex() { return vertex; }
};

class ReadVerilatorXML {
private:
  Graph &netlist;
  std::vector<File> &files;
  std::vector<std::shared_ptr<DType>> &dtypes;
  std::map<std::string, VertexID> vars;
  std::map<std::string, std::shared_ptr<File>> fileIdMappings;
  std::map<std::string, std::shared_ptr<DType>> dtypeMappings;
  std::stack<std::unique_ptr<LogicNode>> logicParents;
  std::stack<std::unique_ptr<ScopeNode>> scopeParents;
  std::unique_ptr<LogicNode> currentLogic;
  std::unique_ptr<ScopeNode> currentScope;
  std::string topName;
  bool isDelayedAssign;
  bool isLValue;

  std::shared_ptr<File> addFile(File file) {
    files.push_back(file);
    return std::make_shared<File>(files.back());
  }
  void addDtype(std::shared_ptr<DType> dtype) {
    dtypes.push_back(dtype);
  }
  std::size_t numChildren(XMLNode *node);
  void dispatchVisitor(XMLNode *node);
  void iterateChildren(XMLNode *node);
  Location parseLocation(const std::string location);
  std::string addTopPrefix(std::string name);
  std::string removeTopPrefix(std::string name);
  VertexID lookupVarVertexExact(const std::string &name);
  VertexID lookupVarVertex(const std::string &name);
  void newVar(XMLNode *node);
  void newScope(XMLNode *node);
  void newVarScope(XMLNode *node);
  void newStatement(XMLNode *node, VertexAstType);
  void newVarRef(XMLNode *node);
  void visitNode(XMLNode *node);
  void visitExtends(XMLNode *node);
  void visitReadMem(XMLNode *node);
  void visitCase(XMLNode *node);
  void visitCaseItem(XMLNode *node);
  void visitEqWild(XMLNode *node);
  void visitGt(XMLNode *node);
  void visitGte(XMLNode *node);
  void visitGtes(XMLNode *node);
  void visitJumpGo(XMLNode *node);
  void visitJumpLabel(XMLNode *node);
  void visitJumpBlock(XMLNode *node);
  void visitLt(XMLNode *node);
  void visitLte(XMLNode *node);
  void visitLtes(XMLNode *node);
  void visitLts(XMLNode *node);
  void visitMul(XMLNode *node);
  void visitMuls(XMLNode *node);
  void visitPowsu(XMLNode *node);
  void visitRedAnd(XMLNode *node);
  void visitRedOr(XMLNode *node);
  void visitRedXor(XMLNode *node);
  void visitReplicate(XMLNode *node);
  void visitShiftls(XMLNode *node);
  void visitShiftrs(XMLNode *node);
  void visitShiftl(XMLNode *node);
  void visitShiftr(XMLNode *node);
  void visitSub(XMLNode *node);
  void visitXor(XMLNode *node);
  void visitAdd(XMLNode *node);
  void visitAnd(XMLNode *node);
  void visitAlways(XMLNode *node);
  void visitArraySel(XMLNode *node);
  void visitAssign(XMLNode *node);
  void visitAssignAlias(XMLNode *node);
  void visitAssignDly(XMLNode *node);
  void visitBasicDtype(XMLNode *node);
  void visitClassRefDType(XMLNode *node);
  void visitConcat(XMLNode *node);
  void visitCond(XMLNode *node);
  void visitCFunc(XMLNode *node);
  void visitCNew(XMLNode *node);
  void visitCMethodCall(XMLNode *node);
  void visitCStmt(XMLNode *node);
  void visitComment(XMLNode *node);
  void visitIf(XMLNode *node);
  void visitExtend(XMLNode *node);
  void visitEq(XMLNode *node);
  void visitGts(XMLNode *node);
  void visitInitial(XMLNode *node);
  void visitInstance(XMLNode *node);
  void visitInterfaceRef(XMLNode *node);
  void visitInterfaceRefDType(XMLNode *node);
  void visitModule(XMLNode *node);
  void visitNeq(XMLNode *node);
  void visitNot(XMLNode *node);
  void visitOr(XMLNode *node);
  void visitRefDtype(XMLNode *node);
  void visitScope(XMLNode *node);
  void visitSel(XMLNode *node);
  void visitSenGate(XMLNode *node);
  void visitSenItem(XMLNode *node);
  void visitSenTree(XMLNode *node);
  void visitText(XMLNode *node);
  void visitTypeTable(XMLNode *node);
  void visitTypedef(XMLNode *node);
  void visitVar(XMLNode *node);
  void visitVarRef(XMLNode *node);
  void visitVarScope(XMLNode *node);
  void visitVoidDType(XMLNode *node);
  void visitWhile(XMLNode *node);
  size_t visitConst(XMLNode *node);
  std::pair<size_t, size_t> visitRange(XMLNode *node);
  MemberDType visitMemberDType(XMLNode *node);
  void visitArrayDType(XMLNode *node, bool packed);
  template<typename T> void visitAggregateDType(XMLNode *node);
  EnumItem visitEnumItem(XMLNode *node);
  void visitEnumDType(XMLNode *node);
  void readXML(const std::string &filename);

public:
  ReadVerilatorXML() = delete;
  ReadVerilatorXML(Graph &netlist,
                   std::vector<File> &files,
                   std::vector<std::shared_ptr<DType>> &dtypes,
                   const std::string &filename);
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_READ_VERILATOR_XML_HPP
