#define BOOST_TEST_MODULE compile_graph

#include <fstream>
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "netlist_paths/CompileGraph.hpp"
#include "netlist_paths/Options.hpp"
#include "tests/definitions.hpp"

namespace fs = boost::filesystem;

constexpr auto VERILOG_SOURCE =
  "module test"
  "  ("
  "    input  logic a,"
  "    input  logic b,"
  "    output logic c"
  "  );"
  "  assign c = a + b;"
  "endmodule";

netlist_paths::Options options;

BOOST_AUTO_TEST_CASE(basic_test) {
  auto inTemp = fs::unique_path();
  auto outTemp = fs::unique_path();
  // Write an input file.
  std::ofstream inFile(inTemp.native());
  inFile << VERILOG_SOURCE;
  inFile.close();
  // Compile the file.
  std::vector<std::string> includes = {};
  std::vector<std::string> defines = {};
  std::vector<std::string> inputFiles = {inTemp.native()};
  netlist_paths::CompileGraph compileGraph(installPrefix);
  BOOST_CHECK_NO_THROW(compileGraph.run(includes,
                                        defines,
                                        inputFiles,
                                        outTemp.native()));
  // Read the output file.
  std::ifstream outFile(outTemp.native());
  std::stringstream outBuffer;
  outBuffer << outFile.rdbuf();
  std::cout << outBuffer.str();
  // Check that the output looks good.
  BOOST_TEST(outBuffer.str().find("digraph netlist {") != std::string::npos);
  BOOST_TEST(outBuffer.str().find("n0[id=0") != std::string::npos);
  BOOST_TEST(outBuffer.str().find("n0 ->") != std::string::npos);
  fs::remove(inTemp);
  fs::remove(outTemp);
}
