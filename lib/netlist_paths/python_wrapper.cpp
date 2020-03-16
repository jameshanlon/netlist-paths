#include <boost/python.hpp>
#include "netlist_paths/Netlist.hpp"
#include "netlist_paths/CompileGraph.hpp"

BOOST_PYTHON_MODULE(py_netlist_paths)
{
  using namespace boost::python;
  using namespace netlist_paths;

  int (CompileGraph::*run)(const std::string&, const std::string&) const = &CompileGraph::run;

  class_<CompileGraph>("CompileGraph")
    .def(init<const std::string&>())
    .def("run", run);

  class_<Netlist>("Netlist")
    .def("parse_file", &Netlist::parseFile)
    .def("reg_exists", &Netlist::regExists);
}
