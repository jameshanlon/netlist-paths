#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Netlist.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/RunVerilator.hpp"

void translateException(const netlist_paths::Exception& e) {
  PyErr_SetString(PyExc_RuntimeError, e.what());
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_vertex_dtype_str_overloads,
                                       getVertexDTypeStr, 1, 2);

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_vertex_dtype_width_overloads,
                                       getVertexDTypeWidth, 1, 2);

BOOST_PYTHON_MODULE(py_netlist_paths)
{
  using namespace boost::python;
  using namespace netlist_paths;

  // Setup exception translation.
  register_exception_translator<Exception>(&translateException);

  class_<Vertex, Vertex*, boost::noncopyable>("Vertex")
     .def("get_name",        &Vertex::getName)
     .def("get_ast_type",    &Vertex::getAstTypeString)
     .def("get_direction",   &Vertex::getDirString)
     .def("get_dtype_str",   &Vertex::getDTypeString)
     .def("get_dtype_width", &Vertex::getDTypeWidth)
     .def("get_location",    &Vertex::getLocString);

  class_<std::vector<Vertex*> >("VertexList")
      .def(vector_indexing_suite<std::vector<Vertex*> >());

  class_<Options, boost::noncopyable>("Options", no_init)
    .def("get_instance",       &Options::getInstancePtr,
                               return_value_policy<reference_existing_object>())
    .staticmethod("get_instance")
    .def("set_match_wildcard", &Options::setMatchWildcard)
    .def("set_match_regex",    &Options::setMatchRegex);

  int (RunVerilator::*run)(const std::string&, const std::string&) const = &RunVerilator::run;

  class_<RunVerilator, boost::noncopyable>("RunVerilator",
                                           init<const std::string&>())
    .def("run", run);

  class_<Netlist, boost::noncopyable>("Netlist",
                                      init<const std::string&>())
    .def("reg_exists",             &Netlist::regExists)
    .def("startpoint_exists",      &Netlist::startpointExists)
    .def("endpoint_exists",        &Netlist::endpointExists)
    .def("path_exists",            &Netlist::pathExists)
    .def("get_any_path",           &Netlist::getAnyPath)
    .def("dump_names",             &Netlist::dumpNamesStdOut)
    .def("get_dtype_width",        &Netlist::getDTypeWidth)
    .def("get_vertex_dtype_str",   &Netlist::getVertexDTypeStr,
                                   get_vertex_dtype_str_overloads())
    .def("get_vertex_dtype_width", &Netlist::getVertexDTypeWidth,
                                   get_vertex_dtype_width_overloads());
}
