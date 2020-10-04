#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Netlist.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/RunVerilator.hpp"
#include "netlist_paths/Waypoints.hpp"

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
     .def("get_ast_type",    &Vertex::getAstTypeStr)
     .def("get_direction",   &Vertex::getDirStr)
     .def("get_dtype_str",   &Vertex::getDTypeStr)
     .def("get_dtype_width", &Vertex::getDTypeWidth)
     .def("get_location",    &Vertex::getLocStr)
     .def("can_ignore",      &Vertex::canIgnore);

  class_<std::vector<Vertex*> >("Path")
      .def(vector_indexing_suite<std::vector<Vertex*> >());

  class_<std::vector<std::vector<Vertex*> > >("PathList")
      .def(vector_indexing_suite<std::vector<std::vector<Vertex*> > >());

  class_<Options, boost::noncopyable>("Options", no_init)
    .def("get_instance",       &Options::getInstancePtr,
                               return_value_policy<reference_existing_object>())
    .staticmethod("get_instance")
    .def("set_verbose",        &Options::setVerbose)
    .def("set_debug",          &Options::setDebug)
    .def("set_match_wildcard", &Options::setMatchWildcard)
    .def("set_match_regex",    &Options::setMatchRegex);

  int (RunVerilator::*run)(const std::string&, const std::string&) const = &RunVerilator::run;

  class_<RunVerilator, boost::noncopyable>("RunVerilator",
                                           init<const std::string&>())
    .def("run", run);

  class_<Waypoints>("Waypoints")
    .def(init<const std::string, const std::string>())
    .def("add_start_point",   &Waypoints::addStartPoint)
    .def("add_finish_point",  &Waypoints::addFinishPoint)
    .def("add_through_point", &Waypoints::addThroughPoint);

  class_<Netlist, boost::noncopyable>("Netlist",
                                      init<const std::string&>())
    .def("get_named_vertices",     &Netlist::getNamedVerticesPtr)
    .def("reg_exists",             &Netlist::regExists)
    .def("startpoint_exists",      &Netlist::startpointExists)
    .def("endpoint_exists",        &Netlist::endpointExists)
    .def("path_exists",            &Netlist::pathExists)
    .def("get_any_path",           &Netlist::getAnyPath)
    .def("get_all_paths",          &Netlist::getAllPaths)
    .def("get_all_fanout_paths",   &Netlist::getAllFanOut)
    .def("get_all_fanin_paths",    &Netlist::getAllFanIn)
    .def("get_dtype_width",        &Netlist::getDTypeWidth)
    .def("get_vertex_dtype_str",   &Netlist::getVertexDTypeStr,
                                   get_vertex_dtype_str_overloads())
    .def("get_vertex_dtype_width", &Netlist::getVertexDTypeWidth,
                                   get_vertex_dtype_width_overloads())
    .def("dump_dot_file",          &Netlist::dumpDotFile);
}
