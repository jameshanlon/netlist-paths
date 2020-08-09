#include <fstream>
#include <iostream>
#include <map>

#include "netlist_paths/DTypes.hpp"
#include "netlist_paths/Debug.hpp"
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/ReadVerilatorXML.hpp"

using namespace netlist_paths;

enum class AstNode {
  ALWAYS,
  ALWAYS_PUBLIC,
  ASSIGN,
  ASSIGN_ALIAS,
  ASSIGN_DLY,
  ASSIGN_W,
  BASIC_DTYPE,
  CONT_ASSIGN,
  CONST,
  C_FUNC,
  INITIAL,
  MODULE,
  PACKED_ARRAY_DTYPE,
  RANGE,
  REF_DTYPE,
  SCOPE,
  SEN_GATE,
  SEN_ITEM,
  STRUCT_DTYPE,
  TOP_SCOPE,
  TYPE_TABLE,
  UNPACKED_ARRAY_DTYPE,
  VAR,
  VAR_REF,
  VAR_SCOPE,
  INVALID
};

/// Convert a string name into an AstNode type.
static AstNode resolveNode(const char *name) {
  static std::map<std::string, AstNode> mappings {
      { "always",             AstNode::ALWAYS },
      { "alwayspublic",       AstNode::ALWAYS_PUBLIC },
      { "assign",             AstNode::ASSIGN },
      { "assignalias",        AstNode::ASSIGN_ALIAS },
      { "assigndly",          AstNode::ASSIGN_DLY },
      { "assignw",            AstNode::ASSIGN_W },
      { "basicdtype",         AstNode::BASIC_DTYPE },
      { "cfunc",              AstNode::C_FUNC },
      { "contassign",         AstNode::CONT_ASSIGN },
      { "const",              AstNode::CONST },
      { "initial",            AstNode::INITIAL },
      { "module",             AstNode::MODULE },
      { "packedarraydtype",   AstNode::PACKED_ARRAY_DTYPE },
      { "refdtype",           AstNode::REF_DTYPE },
      { "scope",              AstNode::SCOPE },
      { "sengate",            AstNode::SEN_GATE },
      { "senitem",            AstNode::SEN_ITEM },
      { "structdtype",        AstNode::STRUCT_DTYPE },
      { "topscope",           AstNode::TOP_SCOPE },
      { "typetable",          AstNode::TYPE_TABLE },
      { "unpackedarraydtype", AstNode::UNPACKED_ARRAY_DTYPE },
      { "var",                AstNode::VAR },
      { "varref",             AstNode::VAR_REF },
      { "varscope",           AstNode::VAR_SCOPE },
  };
  auto it = mappings.find(name);
  return (it != mappings.end()) ? it->second : AstNode::INVALID;
}

void ReadVerilatorXML::dispatchVisitor(XMLNode *node) {
  // Handle node by type.
  switch (resolveNode(node->name())) {
  case AstNode::ALWAYS:               visitAlways(node);             break;
  case AstNode::ALWAYS_PUBLIC:        visitAlways(node);             break;
  case AstNode::ASSIGN:               visitAssign(node);             break;
  case AstNode::ASSIGN_ALIAS:         visitAssign(node);             break;
  case AstNode::ASSIGN_DLY:           visitAssignDly(node);          break;
  case AstNode::ASSIGN_W:             visitAssign(node);             break;
  case AstNode::BASIC_DTYPE:          visitBasicDtype(node);         break;
  case AstNode::CONT_ASSIGN:          visitAssign(node);             break;
  case AstNode::C_FUNC:               visitCFunc(node);              break;
  case AstNode::INITIAL:              visitInitial(node);            break;
  case AstNode::PACKED_ARRAY_DTYPE:   visitArrayDtype(node, true);   break;
  case AstNode::REF_DTYPE:            visitRefDtype(node);           break;
  case AstNode::SCOPE:                visitScope(node);              break;
  case AstNode::SEN_GATE:             visitSenGate(node);            break;
  case AstNode::SEN_ITEM:             visitSenItem(node);            break;
  case AstNode::STRUCT_DTYPE:         visitStructDtype(node);        break;
  case AstNode::TOP_SCOPE:            visitScope(node);              break;
  case AstNode::UNPACKED_ARRAY_DTYPE: visitArrayDtype(node, false);  break;
  case AstNode::VAR:                  visitVar(node);                break;
  case AstNode::VAR_REF:              visitVarRef(node);             break;
  case AstNode::VAR_SCOPE:            visitVarScope(node);           break;
  default:                            visitNode(node);               break;
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
  DEBUG(std::cout << "New scope\n");
  scopeParents.push(std::move(currentScope));
  currentScope = std::make_unique<ScopeNode>(node);
  iterateChildren(node);
  currentScope = std::move(scopeParents.top());
  scopeParents.pop();
}

VertexDesc ReadVerilatorXML::lookupVarVertex(const std::string &name) {
  // Check the var ref is a suffix of a VAR_SCOPE.
  // (This is simplistic and should be improved, and/or check added for multiple matches.)
  auto equals = [&name](const std::unique_ptr<VarNode> &node) {
      return boost::algorithm::ends_with(node->getName(), name); };
  auto it = std::find_if(std::begin(vars), std::end(vars), equals);
  return (it != std::end(vars)) ? (*it)->getVertex()
                                : netlist.nullVertex();
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
  auto name = node->first_attribute("name")->value();
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
  auto vertex = netlist.addVarVertex(VertexAstType::VAR,
                                     direction,
                                     location,
                                     dtypeMappings[dtypeID],
                                     name,
                                     isParam,
                                     paramValue,
                                     isPublic);
  vars.push_back(std::make_unique<VarNode>(name, vertex));
  DEBUG(std::cout << "Add var '" << name << "' to scope\n");
  // Add edges between public/top-level vars and their internal instances
  // eg i_clk and <module>.i_clk (to work around the flattened representation
  // of the netlist.
  if (node->first_attribute("origName")) {
    auto origName = node->first_attribute("origName")->value();
    auto publicVertex = netlist.getVertexDesc(origName);
    if (publicVertex != netlist.nullVertex() && !isParam) {
      assert(netlist.getVertex(publicVertex).isPort() &&
       "expect original-named var to be a port");
      netlist.addEdge(publicVertex, vertex);
      netlist.addEdge(vertex, publicVertex);
      // The direction attribute is only on the top-level/public var, so copy it
      // onto the prefixed version so that they are both identified as ports.
      netlist.setVertexDirection(vertex, netlist.getVertex(publicVertex).getDirection());
      DEBUG(std::cout << "Edge to/from original var "
          << netlist.getVertex(publicVertex).toString() << " to "
          << netlist.getVertex(vertex).toString() << "\n");
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
  DEBUG(std::cout << "New statement: " << getVertexAstTypeStr(vertexType) << "\n");
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
      DEBUG(std::cout << "Edge from parent logic to "
                      << getVertexAstTypeStr(vertexType) << "\n");
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
      throw Exception(std::string("var ")+name+" not under a logic block");
    }
    auto varName = node->first_attribute("name")->value();
    auto varVertex = lookupVarVertex(varName);
    if (varVertex == netlist.nullVertex()) {
      throw Exception(std::string("var ")+varName+" does not have a VAR_SCOPE");
    }
    if (isLValue) {
      // Assignment to var
      if (isDelayedAssign) {
        // Var is reg l-value.
        netlist.addEdge(currentLogic->getVertex(), varVertex);
        netlist.setVertexDstReg(varVertex);
        DEBUG(std::cout << "Edge from LOGIC to REG '" << varName << "'\n");
      } else {
        // Var is wire l-value.
        netlist.addEdge(currentLogic->getVertex(), varVertex);
        DEBUG(std::cout << "Edge from LOGIC to VAR '" << varName << "'\n");
      }
    } else {
      // Var is wire r-value.
      netlist.addEdge(varVertex, currentLogic->getVertex());
      DEBUG(std::cout << "Edge from VAR '" << varName << "' to LOGIC\n");
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

void ReadVerilatorXML::visitModule(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::visitScope(XMLNode *node) {
  newScope(node);
}

void ReadVerilatorXML::visitAssign(XMLNode *node) {
  newStatement(node, VertexAstType::ASSIGN);
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

void ReadVerilatorXML::visitSenItem(XMLNode *node) {
  if (currentLogic) {
    iterateChildren(node);
  } else {
    newStatement(node, VertexAstType::SEN_ITEM);
  }
}

void ReadVerilatorXML::visitSenGate(XMLNode *node) {
  newStatement(node, VertexAstType::SEN_GATE);
}

void ReadVerilatorXML::visitCFunc(XMLNode *node) {
  newStatement(node, VertexAstType::C_FUNC);
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

void ReadVerilatorXML::visitTypeTable(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::visitBasicDtype(XMLNode *node) {
  auto id = node->first_attribute("id")->value();
  auto name = node->first_attribute("name")->value();
  auto location = parseLocation(node->first_attribute("loc")->value());
  if (node->first_attribute("left") && node->first_attribute("right")) {
    auto left = std::stoul(node->first_attribute("left")->value());
    auto right = std::stoul(node->first_attribute("right")->value());
    dtypeMappings[id] = addDtype(BasicDType(name, location, left, right));
  } else {
    dtypeMappings[id] = addDtype(BasicDType(name, location));
  }
}

void ReadVerilatorXML::visitRefDtype(XMLNode *node) {
  auto id = node->first_attribute("id")->value();
  auto name = node->first_attribute("name")->value();
  auto location = parseLocation(node->first_attribute("loc")->value());
  dtypeMappings[id] = addDtype(RefDType(name, location));
}

std::string ReadVerilatorXML::visitConst(XMLNode *node) {
  return node->first_attribute("name")->value();
}

std::pair<std::string, std::string> ReadVerilatorXML::visitRange(XMLNode *node) {
  assert(numChildren(node) == 2 && "range expects two const children");
  auto start = visitConst(node->first_node());
  auto end = visitConst(node->last_node());
  return std::make_pair(start, end);
}

void ReadVerilatorXML::visitArrayDtype(XMLNode *node, bool packed) {
  auto id = node->first_attribute("id")->value();
  auto name = node->first_attribute("name")->value();
  auto location = parseLocation(node->first_attribute("loc")->value());
  assert(numChildren(node) == 1 && "arraydtype expects one range child");
  auto range = visitRange(node->first_node());
  dtypeMappings[id] = addDtype(ArrayDType(name, location,
                                          range.first,
                                          range.second,
                                          packed));
}

void ReadVerilatorXML::visitStructDtype(XMLNode *node) {
  iterateChildren(node);
}

void ReadVerilatorXML::readXML(const std::string &filename) {
  INFO(std::cout << "Parsing input XML file\n");
  std::fstream inputFile(filename);
  if (!inputFile.is_open()) {
    throw Exception("could not open file");
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
  assert(numChildren(netlistNode) == 2 &&
         "expected module and typetable children");
  // Typetable.
  XMLNode *typeTableNode = netlistNode->first_node("typetable");
  visitTypeTable(typeTableNode);
  // Module (single instance).
  XMLNode *topModuleNode = netlistNode->first_node("module");
  visitModule(topModuleNode);
  assert(std::string(topModuleNode->first_attribute("name")->value()) == "TOP" &&
         "top module name does not equal TOP");
  INFO(std::cout << "Netlist contains " << netlist.numVertices()
                 << " vertices and " << netlist.numEdges() << " edges\n");
}

ReadVerilatorXML::ReadVerilatorXML(Netlist &netlist,
                                   std::vector<File> &files,
                                   std::vector<DType> &dtypes,
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
