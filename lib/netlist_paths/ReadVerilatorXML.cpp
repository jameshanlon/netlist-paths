#include <fstream>
#include <iostream>
#include <map>
#include <boost/format.hpp>

#include "netlist_paths/DTypes.hpp"
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/ReadVerilatorXML.hpp"

using namespace netlist_paths;

enum class AstNode {
  ADD,
  ALWAYS,
  ALWAYS_PUBLIC,
  AND,
  ARRAY_SEL,
  ASSIGN,
  ASSIGN_ALIAS,
  ASSIGN_DLY,
  ASSIGN_W,
  BASIC_DTYPE,
  CASE,
  CASE_ITEM,
  CLASS_REF_DTYPE,
  COMMENT,
  CONCAT,
  COND,
  CONST,
  CONT_ASSIGN,
  C_FUNC,
  C_METHOD_CALL,
  C_NEW,
  C_STMT,
  ENUM_DTYPE,
  EQ,
  EQWILD,
  EXTEND,
  EXTENDS,
  GT,
  GTE,
  GTES,
  GTS,
  IF,
  IFACE_REF_DTYPE,
  INITIAL,
  INSTANCE,
  INTF_REF,
  JUMP_BLOCK,
  JUMP_GO,
  JUMP_LABEL,
  LT,
  LTE,
  LTES,
  LTS,
  MEMBER_DTYPE,
  MODULE,
  MUL,
  MULS,
  NEGATE,
  NEQ,
  NOT,
  OR,
  PACKED_ARRAY,
  POWSU,
  RANGE,
  READ_MEM,
  REDAND,
  REDOR,
  REDXOR,
  REF_DTYPE,
  REPLICATE,
  SCOPE,
  SEL,
  SEN_GATE,
  SEN_ITEM,
  SEN_TREE,
  SHIFTL,
  SHIFTLS,
  SHIFTR,
  SHIFTRS,
  STRUCT_DTYPE,
  SUB,
  TEXT,
  TOP_SCOPE,
  TYPEDEF,
  TYPE_TABLE,
  UNION_DTYPE,
  UNPACKED_ARRAY,
  VAR,
  VAR_REF,
  VAR_SCOPE,
  VOID_DTYPE,
  WHILE,
  XOR,
  INVALID
};

/// Convert a string name into an AstNode type.
static AstNode resolveNode(const char *name) {
  static std::map<std::string, AstNode> mappings {
      { "add",              AstNode::ADD },
      { "always",           AstNode::ALWAYS },
      { "alwayspublic",     AstNode::ALWAYS_PUBLIC },
      { "and",              AstNode::AND },
      { "arraysel",         AstNode::ARRAY_SEL },
      { "assign",           AstNode::ASSIGN },
      { "assignalias",      AstNode::ASSIGN_ALIAS },
      { "assigndly",        AstNode::ASSIGN_DLY },
      { "assignw",          AstNode::ASSIGN_W },
      { "basicdtype",       AstNode::BASIC_DTYPE },
      { "case",             AstNode::CASE },
      { "caseitem",         AstNode::CASE_ITEM },
      { "cfunc",            AstNode::C_FUNC },
      { "classrefdtype",    AstNode::CLASS_REF_DTYPE },
      { "cmethodcall",      AstNode::C_METHOD_CALL },
      { "cnew",             AstNode::C_NEW },
      { "comment",          AstNode::COMMENT },
      { "concat",           AstNode::CONCAT },
      { "cond",             AstNode::COND },
      { "const",            AstNode::CONST },
      { "contassign",       AstNode::CONT_ASSIGN },
      { "cstmt",            AstNode::C_STMT },
      { "enumdtype",        AstNode::ENUM_DTYPE },
      { "eq",               AstNode::EQ },
      { "eqwild",           AstNode::EQWILD },
      { "extend",           AstNode::EXTEND },
      { "extends",          AstNode::EXTENDS },
      { "gt",               AstNode::GT },
      { "gte",              AstNode::GTE },
      { "gtes",             AstNode::GTES },
      { "gts",              AstNode::GTS },
      { "if",               AstNode::IF },
      { "ifacerefdtype",    AstNode::IFACE_REF_DTYPE },
      { "initial",          AstNode::INITIAL },
      { "instance",         AstNode::INSTANCE },
      { "intfref",          AstNode::INTF_REF },
      { "jumpblock",        AstNode::JUMP_BLOCK },
      { "jumpgo",           AstNode::JUMP_GO },
      { "jumplabel",        AstNode::JUMP_LABEL },
      { "lt",               AstNode::LT },
      { "lte",              AstNode::LTE },
      { "ltes",             AstNode::LTES },
      { "lts",              AstNode::LTS },
      { "memberdtype",      AstNode::MEMBER_DTYPE },
      { "module",           AstNode::MODULE },
      { "mul",              AstNode::MUL },
      { "muls",             AstNode::MULS },
      { "negate",           AstNode::NEGATE },
      { "neq",              AstNode::NEQ },
      { "not",              AstNode::NOT },
      { "or",               AstNode::OR, },
      { "packarraydtype",   AstNode::PACKED_ARRAY },
      { "powsu",            AstNode::POWSU },
      { "readmem",          AstNode::READ_MEM },
      { "redand",           AstNode::REDAND },
      { "redor",            AstNode::REDOR },
      { "redxor",           AstNode::REDXOR },
      { "refdtype",         AstNode::REF_DTYPE },
      { "replicate",        AstNode::REPLICATE },
      { "scope",            AstNode::SCOPE },
      { "sel",              AstNode::SEL },
      { "sengate",          AstNode::SEN_GATE },
      { "senitem",          AstNode::SEN_ITEM },
      { "sentree",          AstNode::SEN_TREE },
      { "shiftl",           AstNode::SHIFTL },
      { "shiftls",          AstNode::SHIFTLS },
      { "shiftr",           AstNode::SHIFTR },
      { "shiftrs",          AstNode::SHIFTRS },
      { "structdtype",      AstNode::STRUCT_DTYPE },
      { "sub",              AstNode::SUB },
      { "text",             AstNode::TEXT },
      { "topscope",         AstNode::TOP_SCOPE },
      { "typedef",          AstNode::TYPEDEF },
      { "typetable",        AstNode::TYPE_TABLE },
      { "uniondtype",       AstNode::UNION_DTYPE },
      { "unpackarraydtype", AstNode::UNPACKED_ARRAY },
      { "var",              AstNode::VAR },
      { "varref",           AstNode::VAR_REF },
      { "varscope",         AstNode::VAR_SCOPE },
      { "voiddtype",        AstNode::VOID_DTYPE },
      { "while",            AstNode::WHILE },
      { "xor",              AstNode::XOR },
  };
  auto it = mappings.find(name);
  return (it != mappings.end()) ? it->second : AstNode::INVALID;
}

void ReadVerilatorXML::dispatchVisitor(XMLNode *node) {
  // Handle node by type.
  switch (resolveNode(node->name())) {
  case AstNode::ADD:             visitNode(node);                        break;
  case AstNode::ALWAYS:          visitAlways(node);                      break;
  case AstNode::ALWAYS_PUBLIC:   visitAlways(node);                      break;
  case AstNode::AND:             visitNode(node);                        break;
  case AstNode::ARRAY_SEL:       visitNode(node);                        break;
  case AstNode::ASSIGN:          visitAssign(node);                      break;
  case AstNode::ASSIGN_ALIAS:    visitAssignAlias(node);                 break;
  case AstNode::ASSIGN_DLY:      visitAssignDly(node);                   break;
  case AstNode::ASSIGN_W:        visitAssign(node);                      break;
  case AstNode::BASIC_DTYPE:     visitBasicDtype(node);                  break;
  case AstNode::CASE:            visitCase(node);                        break;
  case AstNode::CASE_ITEM:       visitNode(node);                        break;
  case AstNode::CLASS_REF_DTYPE: visitClassRefDType(node);               break;
  case AstNode::COMMENT:         visitNode(node);                        break;
  case AstNode::CONCAT:          visitNode(node);                        break;
  case AstNode::COND:            visitNode(node);                        break;
  case AstNode::CONST:           visitNode(node);                        break; // Don't visit consts unless expected
  case AstNode::CONT_ASSIGN:     visitAssign(node);                      break;
  case AstNode::C_FUNC:          visitCFunc(node);                       break;
  case AstNode::C_METHOD_CALL:   visitCMethodCall(node);                 break;
  case AstNode::C_NEW:           visitCNew(node);                        break;
  case AstNode::C_STMT:          visitCStmt(node);                       break;
  case AstNode::ENUM_DTYPE:      visitEnumDType(node);                   break;
  case AstNode::EQ:              visitNode(node);                        break;
  case AstNode::EQWILD:          visitNode(node);                        break;
  case AstNode::EXTEND:          visitNode(node);                        break;
  case AstNode::EXTENDS:         visitNode(node);                        break;
  case AstNode::GT:              visitNode(node);                        break;
  case AstNode::GTE:             visitNode(node);                        break;
  case AstNode::GTES:            visitNode(node);                        break;
  case AstNode::GTS:             visitNode(node);                        break;
  case AstNode::IF:              visitIf(node);                          break;
  case AstNode::IFACE_REF_DTYPE: visitInterfaceRefDType(node);           break;
  case AstNode::INITIAL:         visitInitial(node);                     break;
  case AstNode::INSTANCE:        visitInstance(node);                    break;
  case AstNode::INTF_REF:        visitInterfaceRef(node);                break;
  case AstNode::JUMP_BLOCK:      visitJumpBlock(node);                   break;
  case AstNode::JUMP_GO:         visitNode(node);                        break;
  case AstNode::JUMP_LABEL:      visitNode(node);                        break;
  case AstNode::LT:              visitNode(node);                        break;
  case AstNode::LTE:             visitNode(node);                        break;
  case AstNode::LTES:            visitNode(node);                        break;
  case AstNode::LTS:             visitNode(node);                        break;
  case AstNode::MEMBER_DTYPE:    visitMemberDType(node);                 break;
  case AstNode::MUL:             visitNode(node);                        break;
  case AstNode::MULS:            visitNode(node);                        break;
  case AstNode::NEGATE:          visitNode(node);                        break;
  case AstNode::NEQ:             visitNode(node);                        break;
  case AstNode::NOT:             visitNode(node);                        break;
  case AstNode::OR:              visitNode(node);                        break;
  case AstNode::PACKED_ARRAY:    visitArrayDType(node, true);            break;
  case AstNode::POWSU:           visitNode(node);                        break;
  case AstNode::READ_MEM:        visitReadMem(node);                     break;
  case AstNode::REDAND:          visitNode(node);                        break;
  case AstNode::REDOR:           visitNode(node);                        break;
  case AstNode::REDXOR:          visitNode(node);                        break;
  case AstNode::REF_DTYPE:       visitRefDtype(node);                    break;
  case AstNode::REPLICATE:       visitNode(node);                        break;
  case AstNode::SCOPE:           visitScope(node);                       break;
  case AstNode::SEL:             visitNode(node);                        break;
  case AstNode::SEN_GATE:        visitNode(node);                        break;
  case AstNode::SEN_ITEM:        visitNode(node);                        break;
  case AstNode::SEN_TREE:        visitNode(node);                        break;
  case AstNode::SHIFTL:          visitNode(node);                        break;
  case AstNode::SHIFTLS:         visitNode(node);                        break;
  case AstNode::SHIFTR:          visitNode(node);                        break;
  case AstNode::SHIFTRS:         visitNode(node);                        break;
  case AstNode::STRUCT_DTYPE:    visitAggregateDType<StructDType>(node); break;
  case AstNode::SUB:             visitNode(node);                        break;
  case AstNode::TEXT:            visitNode(node);                        break;
  case AstNode::TOP_SCOPE:       visitScope(node);                       break;
  case AstNode::TYPEDEF:         visitTypedef(node);                     break;
  case AstNode::UNION_DTYPE:     visitAggregateDType<UnionDType>(node);  break;
  case AstNode::UNPACKED_ARRAY:  visitArrayDType(node, false);           break;
  case AstNode::VAR:             visitVar(node);                         break;
  case AstNode::VAR_REF:         visitVarRef(node);                      break;
  case AstNode::VAR_SCOPE:       visitVarScope(node);                    break;
  case AstNode::VOID_DTYPE:      visitVoidDType(node);                   break;
  case AstNode::WHILE:           visitWhile(node);                       break;
  case AstNode::XOR:             visitNode(node);                        break;
  default:
    //throw XMLException(std::string("Unrecognised node ")+node->name());
    std::cout<<std::string("Unrecognised node ")+node->name()<<"\n";
    //BOOST_LOG_TRIVIAL(debug) << "Unrecognised node: " << node->name();
    //visitNode(node);
    //break;
  }
}

std::size_t ReadVerilatorXML::numChildren(XMLNode *node) {
  std::size_t count = 0;
  for (XMLNode *child = node->first_node();
       child; child = child->next_sibling()) {
    count++;
  }
  return count;
}

void ReadVerilatorXML::iterateChildren(XMLNode *node) {
  for (XMLNode *child = node->first_node();
       child; child = child->next_sibling()) {
    dispatchVisitor(child);
  }
}

void ReadVerilatorXML::newScope(XMLNode *node) {
  BOOST_LOG_TRIVIAL(debug) << "New scope";
  scopeParents.push(std::move(currentScope));
  currentScope = std::make_unique<ScopeNode>(node);
  iterateChildren(node);
  currentScope = std::move(scopeParents.top());
  scopeParents.pop();
}

/// Canonicalise a name by adding the top prefix '<module_name>.' if it is not
/// already a prefix. This is used for associating variable declarations with
/// references.
std::string ReadVerilatorXML::addTopPrefix(std::string name) {
  if (!topName.empty() && name.rfind(topName, 0) == std::string::npos) {
    return topName + "." + name;
  }
  return name;
}

std::string ReadVerilatorXML::removeTopPrefix(std::string name) {
  size_t pos = name.rfind(topName, 0);
  if (!topName.empty() && pos == 0) {
    return name.substr(0, pos+1);
  }
  return name;
}

VertexID ReadVerilatorXML::lookupVarVertexExact(const std::string &name) {
  // Lookup the vertex name directly.
  if (vars.count(name)) {
    return vars[name];
  }
  // Not found.
  return netlist.nullVertex();
}

VertexID ReadVerilatorXML::lookupVarVertex(const std::string &name) {
  // Lookup the vertex name directly.
  if (vars.count(name)) {
    return vars[name];
  }
  // Try to add the top suffix.
  auto extendedName = addTopPrefix(name);
  if (vars.count(extendedName)) {
    return vars[extendedName];
  }
  // Not found.
  return netlist.nullVertex();
}

Location ReadVerilatorXML::parseLocation(const std::string location) {
  std::vector<std::string> tokens;
  boost::split(tokens, location, boost::is_any_of(","));
  auto fileId    = tokens[0];
  auto file      = fileIdMappings[fileId];
  auto startLine = static_cast<unsigned>(std::stoul(tokens[1]));
  auto endLine   = static_cast<unsigned>(std::stoul(tokens[3]));
  auto startCol  = static_cast<unsigned>(std::stoul(tokens[2]));
  auto endCol    = static_cast<unsigned>(std::stoul(tokens[4]));
  return Location(file, startLine, startCol, endLine, endCol);
}

void ReadVerilatorXML::newVar(XMLNode *node) {
  // Create a new vertex from this <var>. Note that since this is a flattened
  // version of the netlist, all <var> nodes occur at the module level, and are
  // followed by a <topscope>, <scope> and then <varscopes>. There is no other
  // scoping in the netlist.
  auto name = std::string(node->first_attribute("name")->value());
  auto location = parseLocation(node->first_attribute("loc")->value());
  auto dtypeID = node->first_attribute("dtype_id")->value();
  auto direction = (node->first_attribute("dir")) ?
                     getVertexDirection(node->first_attribute("dir")->value()) :
                     VertexDirection::NONE;
  auto isParam = false;
  auto paramValue = std::string();
  if (node->first_attribute("param")) {
    assert(std::string(node->first_node()->name()) == "const" &&
           "expect const node under param");
    isParam = true;
    paramValue = node->first_node()->first_attribute("name")->value();
  }
  auto isPublic = node->first_attribute("public") != nullptr;

  // Determine the top name of the by inspecting the prefixes of names, outside
  // of the main scope. Ideally the top name would be specified in the netlist
  // XML, but this is a solution until it is.
  if (scopeParents.empty()) {
    size_t pos = name.find_first_of('.');
    // If it has a dot prefix, and that isn't a Verilator generated name.
    if (pos != std::string::npos &&
        name.rfind("__V", 0) == std::string::npos) {
      if (topName.empty()) {
        topName = name.substr(0, pos);
        BOOST_LOG_TRIVIAL(debug) << "Got top name " << topName;
      } else {
        assert(topName == name.substr(0, pos) && "all name prefixes should match the top name");
      }
    }
  }

  // Canonicalise the variable name by adding a top prefix if it is known.
  auto canonicalName = addTopPrefix(name);
  auto vertex = netlist.addVarVertex(VertexAstType::VAR, direction, location,
                                     dtypeMappings[dtypeID], canonicalName,
                                     isParam, paramValue, isPublic);
  if (vars.count(canonicalName) == 0) {
    vars[canonicalName] = vertex;
    BOOST_LOG_TRIVIAL(debug) << boost::format("Add var %s (canonical %s) to scope") % name % canonicalName;
  } else {
    BOOST_LOG_TRIVIAL(debug) << boost::format("Var %s (canonical %s) already exists") % name % canonicalName;
  }

  // Add edges between public/top-level port variables and their internal
  // instances eg i_clk and <module>.i_clk. This is a work around the flattened
  // representation of the netlist.
  if (node->first_attribute("origName")) {
    auto origName = node->first_attribute("origName")->value();
    auto publicVertex = lookupVarVertexExact(origName);
    if (publicVertex != netlist.nullVertex() &&
        publicVertex != vertex &&
        netlist.getVertex(publicVertex).isPort() &&
        !isParam) {
      netlist.addEdge(publicVertex, vertex);
      netlist.addEdge(vertex, publicVertex);
      // The direction attribute is only on the top-level/public var, so copy it
      // onto the prefixed version so that they are both identified as ports.
      netlist.setVertexDirection(vertex, netlist.getVertex(publicVertex).getDirection());
      BOOST_LOG_TRIVIAL(debug) << "Edge to/from original var "
                               << netlist.getVertex(publicVertex).toString() << " to "
                               << netlist.getVertex(vertex).toString();
    }
  }
}

void ReadVerilatorXML::newVarScope(XMLNode *node) {
  auto name = node->first_attribute("name")->value();
  auto existingVarVertex = lookupVarVertex(name);
  // newVar is called for 'var' and 'varscope' nodes since Verilator introduces
  // some nodes during its transformations on as 'varscope's.
  if (existingVarVertex == netlist.nullVertex()) {
    newVar(node);
  }
}

void ReadVerilatorXML::newStatement(XMLNode *node, VertexAstType vertexType) {
  BOOST_LOG_TRIVIAL(debug) << "New statement: " << getVertexAstTypeStr(vertexType);
  // A statment must have a scope for variable references to occur in.
  if (currentScope) {
    logicParents.push(std::move(currentLogic));
    // Create a vertex for this logic.
    auto location = parseLocation(node->first_attribute("loc")->value());
    auto vertex = netlist.addLogicVertex(vertexType, location);
    currentLogic = std::make_unique<LogicNode>(node, *currentScope, vertex);
    // Create an edge from the parent logic to this one.
    if (logicParents.top()) {
      auto vertexParent = logicParents.top()->getVertex();
      netlist.addEdge(vertexParent, vertex);
      BOOST_LOG_TRIVIAL(debug) << "Edge from parent logic to "
                               << getVertexAstTypeStr(vertexType);
    }
    if (vertexType == VertexAstType::ASSIGN ||
        vertexType == VertexAstType::ASSIGN_ALIAS ||
        vertexType == VertexAstType::ASSIGN_DLY ||
        vertexType == VertexAstType::ASSIGN_W) {
      // Handle assignments to distinguish L and R values.
      assert(numChildren(node) == 2 &&
             "assign statement has more than 2 children");
      dispatchVisitor(node->first_node()); // R-value.
      isLValue = true;
      dispatchVisitor(node->last_node()); // L-value.
      isLValue = false;
    } else {
      iterateChildren(node);
    }
    currentLogic = std::move(logicParents.top());
    logicParents.pop();
  }
}

void ReadVerilatorXML::newVarRef(XMLNode *node) {
  if (currentScope) {
    if (!currentLogic) {
      auto name = std::string(node->first_attribute("name")->value());
      throw XMLException(std::string("var ")+name+" not under a logic block");
    }
    auto varName = node->first_attribute("name")->value();
    auto varVertex = lookupVarVertex(varName);
    if (varVertex == netlist.nullVertex()) {
      // Workaround issue #7: https://github.com/jameshanlon/netlist-paths/issues/7
      BOOST_LOG_TRIVIAL(warning) << "var " << +varName << " does not have a VAR_SCOPE";
      return;
    }
    if (isLValue) {
      // Assignment to var
      if (isDelayedAssign) {
        // Var is reg l-value.
        netlist.addEdge(currentLogic->getVertex(), varVertex);
        netlist.setVertexDstReg(varVertex);
        BOOST_LOG_TRIVIAL(debug) << "Edge from LOGIC to REG " << varName;
      } else {
        // Var is wire l-value.
        netlist.addEdge(currentLogic->getVertex(), varVertex);
        BOOST_LOG_TRIVIAL(debug) << "Edge from LOGIC to VAR " << varName;
      }
    } else {
      // Var is wire r-value.
      netlist.addEdge(varVertex, currentLogic->getVertex());
      BOOST_LOG_TRIVIAL(debug) << "Edge from VAR " << varName << " to LOGIC";
    }
    iterateChildren(node);
  }
}

//===----------------------------------------------------------------------===//
// Visitor methods.
//===----------------------------------------------------------------------===//

void ReadVerilatorXML::visitNode(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::visitReadMem(XMLNode *node) {
  newStatement(node, VertexAstType::READ_MEM);
}

void ReadVerilatorXML::visitCase(XMLNode *node) {
  newStatement(node, VertexAstType::CASE);
}

void ReadVerilatorXML::visitJumpBlock(XMLNode *node) {
  newStatement(node, VertexAstType::JUMP_BLOCK);
}

void ReadVerilatorXML::visitModule(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::visitScope(XMLNode *node) {
  newScope(node);
}

void ReadVerilatorXML::visitAssign(XMLNode *node) {
  newStatement(node, VertexAstType::ASSIGN);
}

void ReadVerilatorXML::visitAssignAlias(XMLNode *node) {
  newStatement(node, VertexAstType::ASSIGN_ALIAS);
}

void ReadVerilatorXML::visitAssignDly(XMLNode *node) {
  isDelayedAssign = true;
  newStatement(node, VertexAstType::ASSIGN_DLY);
  isDelayedAssign = false;
}

void ReadVerilatorXML::visitAlways(XMLNode *node) {
  newStatement(node, VertexAstType::ALWAYS);
}

void ReadVerilatorXML::visitInitial(XMLNode *node) {
  newStatement(node, VertexAstType::INITIAL);
}

void ReadVerilatorXML::visitIf(XMLNode *node) {
  newStatement(node, VertexAstType::IF);
}

void ReadVerilatorXML::visitInstance(XMLNode *node) {
  newStatement(node, VertexAstType::INSTANCE);
}

void ReadVerilatorXML::visitSenGate(XMLNode *node) {
  newStatement(node, VertexAstType::SEN_GATE);
}

void ReadVerilatorXML::visitCFunc(XMLNode *node) {
  newStatement(node, VertexAstType::C_FUNC);
}

void ReadVerilatorXML::visitCNew(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::visitCMethodCall(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::visitCStmt(XMLNode *node) {
  newStatement(node, VertexAstType::C_STMT);
}

void ReadVerilatorXML::visitVar(XMLNode *node) {
  newVar(node);
}

void ReadVerilatorXML::visitVarScope(XMLNode *node) {
  newVarScope(node);
}

void ReadVerilatorXML::visitVarRef(XMLNode *node) {
  newVarRef(node);
  iterateChildren(node);
}

void ReadVerilatorXML::visitInterfaceRef(XMLNode *node) {
  // No newVarRef(node); since interfaces are not supported in flat netlists.
  iterateChildren(node);
}

void ReadVerilatorXML::visitTypeTable(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::visitTypedef(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::visitWhile(XMLNode *node) {
  newStatement(node, VertexAstType::WHILE);
}

void ReadVerilatorXML::visitBasicDtype(XMLNode *node) {
  auto id = node->first_attribute("id")->value();
  if (dtypeMappings.count(id) == 0) {
    auto name = node->first_attribute("name")->value();
    auto location = parseLocation(node->first_attribute("loc")->value());
    if (node->first_attribute("left") && node->first_attribute("right")) {
      auto left = std::stoul(node->first_attribute("left")->value());
      auto right = std::stoul(node->first_attribute("right")->value());
      dtypeMappings[id] = std::make_shared<BasicDType>(name, location, left, right);
    } else {
      dtypeMappings[id] = std::make_shared<BasicDType>(name, location);
    }
    addDtype(dtypeMappings[id]);
  }
}

void ReadVerilatorXML::visitRefDtype(XMLNode *node) {
  auto id = node->first_attribute("id")->value();
  auto subDTypeId = node->first_attribute("sub_dtype_id")->value();
  if (dtypeMappings.count(id) == 0) {
    auto name = node->first_attribute("name")->value();
    auto location = parseLocation(node->first_attribute("loc")->value());
    dtypeMappings[id] = std::make_shared<RefDType>(name, location);
    addDtype(dtypeMappings[id]);
  } else {
    // Second pass (sub DType declaration can occur after).
    if (dtypeMappings.count(subDTypeId) == 0) {
      throw XMLException(std::string("could not find ref sub dtype ID ")+subDTypeId);
    }
    dynamic_cast<RefDType*>(dtypeMappings[id].get())->setSubDType(dtypeMappings[subDTypeId]);
  }
}

MemberDType ReadVerilatorXML::visitMemberDType(XMLNode *node) {
  auto name = node->first_attribute("name")->value();
  auto location = parseLocation(node->first_attribute("loc")->value());
  auto subDTypeId = node->first_attribute("sub_dtype_id")->value();
  if (dtypeMappings.count(subDTypeId) == 0) {
    throw XMLException(std::string("could not find member sub dtype ID ")+subDTypeId);
  }
  return MemberDType(name, location, dtypeMappings[subDTypeId]);
}

size_t ReadVerilatorXML::visitConst(XMLNode *node) {
  auto value = std::string(node->first_attribute("name")->value());
  if (value.rfind("'") != std::string::npos) {
    // Drop any value type prefixes.
    auto pos = value.rfind("'sh");
    if (pos != std::string::npos) {
      return std::stoll(value.substr(pos+3), nullptr, 16);
    }
    pos = value.rfind("'h");
    if (pos != std::string::npos) {
      return std::stoull(value.substr(pos+2), nullptr, 16);
    }
    assert(0 && "Unexpected constant type prefix");
  }
  return std::stoull(value);
}

std::pair<size_t, size_t>
ReadVerilatorXML::visitRange(XMLNode *node) {
  assert(numChildren(node) == 2 && "range expects two const children");
  auto start = visitConst(node->last_node());
  auto end = visitConst(node->first_node());
  return std::make_pair(start, end);
}

void ReadVerilatorXML::visitArrayDType(XMLNode *node, bool packed) {
  auto id = node->first_attribute("id")->value();
  auto subDTypeId = node->first_attribute("sub_dtype_id")->value();
  if (dtypeMappings.count(id) == 0) {
    auto location = parseLocation(node->first_attribute("loc")->value());
    assert(numChildren(node) == 1 && "arraydtype expects one range child");
    auto range = visitRange(node->first_node());
    dtypeMappings[id] = std::make_shared<ArrayDType>(location,
                                                     range.first,
                                                     range.second,
                                                     packed);
    addDtype(dtypeMappings[id]);
  } else {
    // Second pass (sub DType declaration can occur after).
    if (dtypeMappings.count(subDTypeId) == 0) {
      throw XMLException(std::string("could not find array sub dtype ID ")+subDTypeId);
    }
    dynamic_cast<ArrayDType*>(dtypeMappings[id].get())->setSubDType(dtypeMappings[subDTypeId]);
  }
}

/// Shared handling for structs and unions.
template<typename T>
void ReadVerilatorXML::visitAggregateDType(XMLNode *node) {
  auto id = node->first_attribute("id")->value();
  if (dtypeMappings.count(id) == 0) {
    auto location = parseLocation(node->first_attribute("loc")->value());
    std::shared_ptr<T> dtype;
    // Struct or union may not be named, and defined inline with a declaration.
    if (node->first_attribute("name")) {
      auto name = node->first_attribute("name")->value();
      dtype = std::make_shared<T>(name, location);
    } else {
      dtype = std::make_shared<T>(location);
    }
    dtypeMappings[id] = dtype;
    addDtype(dtype);
  } else {
    // Second pass to resolve sub DTypes.
    for (XMLNode *child = node->first_node();
         child; child = child->next_sibling()) {
      assert(std::string(child->name()) == "memberdtype" &&
             "aggregate dtype expects memberdtype children");
      dynamic_cast<T*>(dtypeMappings[id].get())->addMemberDType(visitMemberDType(child));
    }
  }
}

EnumItem ReadVerilatorXML::visitEnumItem(XMLNode *node) {
  auto name = node->first_attribute("name")->value();
  auto value = visitConst(node->first_node());
  return EnumItem(name, value);
}

void ReadVerilatorXML::visitEnumDType(XMLNode *node) {
  auto id = node->first_attribute("id")->value();
  auto subDTypeId = node->first_attribute("sub_dtype_id")->value();
  if (dtypeMappings.count(id) == 0) {
    auto location = parseLocation(node->first_attribute("loc")->value());
    auto name = node->first_attribute("name")->value();
    auto dtype = std::make_shared<EnumDType>(name, location);
    for (XMLNode *child = node->first_node();
         child; child = child->next_sibling()) {
      assert(std::string(child->name()) == "enumitem" &&
             "enumdtype expects enumitem children");
      dtype->addItem(visitEnumItem(child));
    }
    dtypeMappings[id] = dtype;
    addDtype(dtype);
  } else {
    // Second pass (sub DType declaration can occur after).
    if (dtypeMappings.count(subDTypeId) == 0) {
      throw XMLException(std::string("could not find enum sub dtype ID ")+subDTypeId);
    }
    dynamic_cast<EnumDType*>(dtypeMappings[id].get())->setSubDType(dtypeMappings[subDTypeId]);
  }
}

void ReadVerilatorXML::visitInterfaceRefDType(XMLNode *node) {
 // To do.
}

void ReadVerilatorXML::visitVoidDType(XMLNode *node) {
  auto id = node->first_attribute("id")->value();
  if (dtypeMappings.count(id) == 0) {
    auto location = parseLocation(node->first_attribute("loc")->value());
    dtypeMappings[id] = std::make_shared<VoidDType>(location);
    addDtype(dtypeMappings[id]);
  }
}

void ReadVerilatorXML::visitClassRefDType(XMLNode *node) {
  auto id = node->first_attribute("id")->value();
  if (dtypeMappings.count(id) == 0) {
    auto name = node->first_attribute("name")->value();
    auto location = parseLocation(node->first_attribute("loc")->value());
    dtypeMappings[id] = std::make_shared<ClassRefDType>(name, location);
    addDtype(dtypeMappings[id]);
  }
}

void ReadVerilatorXML::readXML(const std::string &filename) {
  BOOST_LOG_TRIVIAL(info) << "Parsing input XML file";
  std::fstream inputFile(filename);
  if (!inputFile.is_open()) {
    throw XMLException("could not open file");
  }
  // Parse the buffered XML.
  rapidxml::xml_document<> doc;
  std::vector<char> buffer((std::istreambuf_iterator<char>(inputFile)),
                            std::istreambuf_iterator<char>());
  buffer.push_back('\0');
  doc.parse<0>(&buffer[0]);
  // Find our root node
  XMLNode *rootNode = doc.first_node("verilator_xml");
  // Files section
  XMLNode *filesNode = rootNode->first_node("files");
  for (XMLNode *fileNode = filesNode->first_node("file");
       fileNode; fileNode = fileNode->next_sibling()) {
    auto fileId = fileNode->first_attribute("id")->value();
    auto filename = fileNode->first_attribute("filename")->value();
    auto language = fileNode->first_attribute("language")->value();
    fileIdMappings[fileId] = addFile(File(filename, language));
  }
  // Netlist section.
  XMLNode *netlistNode = rootNode->first_node("netlist");
  // Count the nubmer of modules.
  size_t moduleCount = 0;
  size_t interfaceCount = 0;
  size_t packageCount = 0;
  for (XMLNode *moduleNode = netlistNode->first_node("module");
       moduleNode; moduleNode = moduleNode->next_sibling()) {
    if (std::string(moduleNode->name()) == "iface") { interfaceCount++; }
    if (std::string(moduleNode->name()) == "module") { moduleCount++; }
    if (std::string(moduleNode->name()) == "package") { packageCount++; }
  }
  BOOST_LOG_TRIVIAL(info) << moduleCount    << " modules in netlist";
  BOOST_LOG_TRIVIAL(info) << interfaceCount << " interfaces in netlist";
  BOOST_LOG_TRIVIAL(info) << packageCount   << " packages in netlist";
  // Typetable (two passes to resolve forward dtype ID references).
  XMLNode *typeTableNode = netlistNode->first_node("typetable");
  visitTypeTable(typeTableNode);
  visitTypeTable(typeTableNode);
  BOOST_LOG_TRIVIAL(info) << boost::format("%d entries in type table") % dtypes.size();
  // Module (single instance).
  if (moduleCount == 1 && interfaceCount == 0) {
    XMLNode *topModuleNode = netlistNode->first_node("module");
    visitModule(topModuleNode);
    if (std::string(topModuleNode->first_attribute("name")->value()) != "TOP") {
      throw XMLException("unexpected top module name");
    }
    BOOST_LOG_TRIVIAL(info) << boost::format("Netlist contains %d vertices and %d edges")
                                 % netlist.numVertices() % netlist.numEdges();
  } else {
    BOOST_LOG_TRIVIAL(info) << "Netlist is not flat, skipping modules";
  }
}

ReadVerilatorXML::ReadVerilatorXML(Graph &netlist,
                                   std::vector<File> &files,
                                   std::vector<std::shared_ptr<DType>> &dtypes,
                                   const std::string &filename) :
    netlist(netlist),
    files(files),
    dtypes(dtypes),
    currentLogic(nullptr),
    currentScope(nullptr),
    isDelayedAssign(false),
    isLValue(false) {
  readXML(filename);
}
