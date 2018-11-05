#define BOOST_TEST_MODULE analyse_graph

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "netlist_paths/AnalyseGraph.hpp"
#include "netlist_paths/Options.hpp"
#include "tests/definitions.hpp"

namespace fs = boost::filesystem;

constexpr auto GRAPH_SOURCE =
  "VERTEX 1 VAR name @ location:0\n"
  "VERTEX 2 VAR name @ location:0\n"
  "VERTEX 3 VAR name @ location:0\n"
  "EDGE 1 -> 2\n"
  "EDGE 2 -> 3\n"
  "EDGE 3 -> 1\n"
  "EDGE 1 -> 3";

netlist_paths::Options options;

BOOST_AUTO_TEST_CASE(parse_input) {
  auto outTemp = fs::unique_path();
  // Write an input file.
  std::ofstream outFile(outTemp.native());
  outFile << GRAPH_SOURCE;
  outFile.close();
  netlist_paths::AnalyseGraph analyseGraph;
  BOOST_CHECK_NO_THROW(analyseGraph.parseFile(outTemp.native()));
  BOOST_TEST(analyseGraph.getNumVertices() == 3);
  BOOST_TEST(analyseGraph.getNumEdges() == 4);
  fs::remove(outTemp);
}
