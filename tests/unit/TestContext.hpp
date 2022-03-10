#ifndef NETLIST_PATHS_TEST_CONTEXT_HPP
#define NETLIST_PATHS_TEST_CONTEXT_HPP

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "netlist_paths/Netlist.hpp"
#include "netlist_paths/RunVerilator.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/Waypoints.hpp"

namespace fs = boost::filesystem;

#define CHECK_VAR_REPORT(vertex, astTypeStr, dtypeStr, name) \
  do { \
    BOOST_TEST(vertex->getAstTypeStr() == astTypeStr); \
    BOOST_TEST(vertex->getDTypeStr() == dtypeStr); \
    BOOST_TEST(vertex->getName() == name); \
  } while (0);

#define CHECK_LOG_REPORT(vertex, astTypeStr) \
  BOOST_TEST(vertex->getAstTypeStr() == astTypeStr)

struct TestContext {

  std::unique_ptr<netlist_paths::Netlist> np;

  TestContext() {}

  /// Check all names are unique.
  void uniqueNames() {
    auto vertices = np->getNamedVertices();
    std::vector<std::string> names;
    for (auto v : vertices) {
      auto name = std::remove_reference<const netlist_paths::Vertex>::type(v).getName();
      name += std::remove_reference<const netlist_paths::Vertex>::type(v).getAstTypeStr();
      names.push_back(name);
    }
    auto last = std::unique(std::begin(names), std::end(names));
    names.erase(last, std::end(names));
    BOOST_TEST(vertices.size() == names.size());
  }

  /// Check all hierarchical names are qualified with the top module name.
  void qualifiedNames(const std::string &topName) {
    for (auto v : np->getNamedVertices()) {
      auto name = std::remove_reference<const netlist_paths::Vertex>::type(v).getName();
      if (name.find('.') != std::string::npos &&
          name.rfind("__", 0) == std::string::npos) {
        BOOST_TEST(boost::starts_with(name, topName));
      }
    }
  }

  void init(const std::string &topName) {
    // Check names.
    uniqueNames();
    qualifiedNames(topName);
    // Default options.
    netlist_paths::Options::getInstance().setErrorOnUnmatchedNode(true);
    netlist_paths::Options::getInstance().setMatchExact();
    netlist_paths::Options::getInstance().setIgnoreHierarchyMarkers(false);
    netlist_paths::Options::getInstance().setMatchOneVertex();
    netlist_paths::Options::getInstance().setTraverseRegisters(false);
    netlist_paths::Options::getInstance().setRestrictStartPoints(true);
    netlist_paths::Options::getInstance().setRestrictEndPoints(true);
  }

  /// Compile a test and create a netlist object.
    void compile(const std::string inFilename, std::string topName="", std::string topModule="") {
        auto testPath = fs::path(testPrefix) / inFilename;
        std::vector<std::string> includes = {};
        std::vector<std::string> defines = {};
        std::vector<std::string> inputFiles = {testPath.string()};
        // Check the Verilator binary exists.
        BOOST_ASSERT(boost::filesystem::exists(installPrefix));
        netlist_paths::RunVerilator runVerilator(installPrefix);
        auto outTemp = fs::unique_path();
        runVerilator.run(includes,
                         defines,
                         inputFiles,
                         topModule,
                         outTemp.native());
        np = std::make_unique<netlist_paths::Netlist>(outTemp.native());
        fs::remove(outTemp);
        if (topName.empty()) {
            topName = boost::filesystem::change_extension(inFilename, "").string();
        }
        init(topName);
    }

  /// Create a netlist object from XML.
  void load(const std::string inFilename, std::string topName="") {
    auto xmlPath = fs::path(xmlPrefix) / inFilename;
    np = std::make_unique<netlist_paths::Netlist>(xmlPath.string());
    if (topName.empty()) {
      topName = boost::filesystem::change_extension(inFilename, "").string();
    }
    init(topName);
  }
};

#endif // NETLIST_PATHS_TEST_CONTEXT_HPP
