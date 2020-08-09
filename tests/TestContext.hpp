#ifndef NETLIST_PATHS_TEST_CONTEXT_HPP
#define NETLIST_PATHS_TEST_CONTEXT_HPP

#include <string>
#include <vector>
#include <boost/test/unit_test.hpp>
#include <netlist_paths/NetlistPaths.hpp>

struct TestContext {
  TestContext() {}
  std::unique_ptr<netlist_paths::NetlistPaths> np;
  /// Compile a test and create a netlist object.
  void compile(const std::string &inFilename,
               const std::string &topName) {
    auto testPath = fs::path(testPrefix) / inFilename;
    std::vector<std::string> includes = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> inputFiles = {testPath.string()};
    netlist_paths::RunVerilator runVerilator(installPrefix);
    auto outTemp = fs::unique_path();
    runVerilator.run(includes,
                     defines,
                     inputFiles,
                     outTemp.native());
    np = std::make_unique<netlist_paths::NetlistPaths>(outTemp.native());
    fs::remove(outTemp);
    uniqueNames();
    qualifiedNames(topName);
  }
  /// Check all names are unique.
  void uniqueNames() {
    auto vertices = np->getNamedVertices();
    std::vector<std::string> names;
    for (auto v : vertices) {
      auto name = std::remove_reference<const netlist_paths::Vertex>::type(v).getName();
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
  bool regExists(const std::string &name) { return np->regExists(name); }
  bool pathExists(const std::string &start,
                  const std::string &end) { return np->pathExists(start, end); }
};

#endif // NETLIST_PATHS_TEST_CONTEXT_HPP
