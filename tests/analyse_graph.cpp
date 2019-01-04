#define BOOST_TEST_MODULE analyse_graph

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "netlist_paths/AnalyseGraph.hpp"
#include "netlist_paths/Options.hpp"
#include "tests/definitions.hpp"

namespace fs = boost::filesystem;

constexpr auto GRAPH_SOURCE =
  "digraph test {"
  "  n0 [id=0 name=\"foo\"];\n"
  "  n1 [id=1 name=\"bar\"];\n"
  "  n2 [id=2 name=\"baz\"];\n"
  "  n0 -> n1;\n"
  "  n1 -> n2;\n"
  "  n2 -> n1;\n"
  "  n2 -> n0;\n"
  "}\n";

netlist_paths::Options options;


BOOST_AUTO_TEST_CASE(parse_input) {
  auto outTemp = fs::unique_path();
  // Write an input file.
  std::ofstream outFile(outTemp.native());
  outFile << GRAPH_SOURCE;
  outFile.close();
  netlist_paths::AnalyseGraph analyseGraph;
  BOOST_CHECK_NO_THROW(analyseGraph.parseFile(outTemp.native()));
  analyseGraph.checkGraph();
  BOOST_TEST(analyseGraph.getNumVertices() == 3);
  BOOST_TEST(analyseGraph.getNumEdges() == 4);
  fs::remove(outTemp);
}

BOOST_AUTO_TEST_CASE(parse_input_boost) {
  options.boostParser = true;
  auto outTemp = fs::unique_path();
  // Write an input file.
  std::ofstream outFile(outTemp.native());
  outFile << GRAPH_SOURCE;
  outFile.close();
  netlist_paths::AnalyseGraph analyseGraph;
  BOOST_CHECK_NO_THROW(analyseGraph.parseFile(outTemp.native()));
  analyseGraph.checkGraph();
  BOOST_TEST(analyseGraph.getNumVertices() == 3);
  BOOST_TEST(analyseGraph.getNumEdges() == 4);
  fs::remove(outTemp);
}
