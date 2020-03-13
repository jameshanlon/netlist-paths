#include <boost/python.hpp>
#include "netlist_paths/AnalyseGraph.hpp"
#include "netlist_paths/CompileGraph.hpp"

BOOST_PYTHON_MODULE(py_netlist_paths)
{
  using namespace boost::python;
  using namespace netlist_paths;

  class_<CompileGraph>("CompileGraph")
    .def(init<const std::string&>())
    .def("run", &CompileGraph::run);

  class_<AnalyseGraph>("AnalyseGraph")
    .def("parseFile", &AnalyseGraph::parseFile);
}
