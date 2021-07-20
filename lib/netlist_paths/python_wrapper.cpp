#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "netlist_paths/DTypes.hpp"
#include "netlist_paths/Exception.hpp"
#include "netlist_paths/Netlist.hpp"
#include "netlist_paths/Options.hpp"
#include "netlist_paths/RunVerilator.hpp"
#include "netlist_paths/Vertex.hpp"
#include "netlist_paths/Path.hpp"
#include "netlist_paths/Waypoints.hpp"

void translateException(const netlist_paths::Exception& e) {
  PyErr_SetString(PyExc_RuntimeError, e.what());
}

void translateXMLException(const netlist_paths::XMLException& e) {
  PyErr_SetString(PyExc_RuntimeError, e.what());
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_named_vertices_overloads,
                                       getNamedVerticesPtr, 0, 1)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_reg_vertices_overloads,
                                       getRegVerticesPtr, 0, 1)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_net_vertices_overloads,
                                       getNetVerticesPtr, 0, 1)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_port_vertices_overloads,
                                       getPortVerticesPtr, 0, 1)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_vertex_dtype_str_overloads,
                                       getVertexDTypeStr, 1, 2)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_vertex_dtype_width_overloads,
                                       getVertexDTypeWidth, 1, 2)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(dtype_to_str_overloads,
                                       toString, 0, 1)

BOOST_PYTHON_MODULE(py_netlist_paths)
{
  using namespace boost::python;
  using namespace netlist_paths;

  // Setup exception translation.
  register_exception_translator<Exception>(&translateException);
  register_exception_translator<XMLException>(&translateXMLException);

  class_<DType, DType*, boost::noncopyable>("DType")
     .def("get_name",   &DType::getName)
     .def("get_width",  &DType::getWidth)
     .def("to_str",     &DType::toString,
                        dtype_to_str_overloads());

  class_<Vertex, Vertex*, boost::noncopyable>("Vertex")
     .def("get_id",            &Vertex::getID)
     .def("get_name",          &Vertex::getName)
     .def("get_ast_type_str",  &Vertex::getSimpleAstTypeStr)
     .def("get_direction_str", &Vertex::getDirStr)
     .def("get_dtype",         &Vertex::getDTypePtr,
                               return_value_policy<reference_existing_object>())
     .def("get_dtype_str",     &Vertex::getDTypeStr)
     .def("get_dtype_width",   &Vertex::getDTypeWidth)
     .def("get_location_str",  &Vertex::getLocationStr)
     .def("is_top",            &Vertex::isTop)
     .def("is_logic",          &Vertex::isLogic)
     .def("is_parameter",      &Vertex::isParameter)
     .def("is_net",            &Vertex::isNet)
     .def("is_reg",            &Vertex::isReg)
     .def("is_port",           &Vertex::isPort)
     .def("is_start_point",    &Vertex::isStartPoint)
     .def("is_end_point",      &Vertex::isEndPoint)
     .def("is_mid_point",      &Vertex::isMidPoint)
     .def("is_public",         &Vertex::isPublic)
     .def("can_ignore",        &Vertex::canIgnore);

  class_<Path>("Path")
    .def("append_vertex",     &Path::appendVertex)
    .def("append_path",       &Path::appendPath)
    .def("contains_vertex",   &Path::contains)
    .def("reverse",           &Path::reverse)
    .def("get_vertices",      &Path::getVerticesNoConst)
    .def("get_vertex",        &Path::getVertexNoConst,
                              return_value_policy<reference_existing_object>())
    .def("get_start_vertex",  &Path::getStartVertexNoConst,
                              return_value_policy<reference_existing_object>())
    .def("get_finish_vertex", &Path::getFinishVertexNoConst,
                              return_value_policy<reference_existing_object>())
    .def("length",            &Path::length)
    .def("empty",             &Path::empty);

  class_<std::vector<Vertex*> >("VertexList")
      .def(vector_indexing_suite<std::vector<Vertex*> >());

  class_<std::vector<Path> >("PathList")
      .def(vector_indexing_suite<std::vector<Path> >());

  class_<std::vector<DType*> >("DType")
      .def(vector_indexing_suite<std::vector<DType*> >());

  class_<Options, boost::noncopyable>("Options", no_init)
    .def("get_instance",                  &Options::getInstancePtr,
                                          return_value_policy<reference_existing_object>())
    .staticmethod("get_instance")
    .def("set_verbose",                   &Options::setVerbose)
    .def("set_debug",                     &Options::setDebug)
    .def("set_match_exact",               &Options::setMatchExact)
    .def("set_match_wildcard",            &Options::setMatchWildcard)
    .def("set_match_regex",               &Options::setMatchRegex)
    .def("set_match_any_vertex",          &Options::setMatchAnyVertex)
    .def("set_match_one_vertex",          &Options::setMatchOneVertex)
    .def("set_traverse_registers",        &Options::setTraverseRegisters)
    .def("set_restrict_start_points",     &Options::setRestrictStartPoints)
    .def("set_restrict_end_points",       &Options::setRestrictEndPoints)
    .def("set_ignore_hierarchy_markers",  &Options::setIgnoreHierarchyMarkers)
    .def("set_error_on_unmatched_node",   &Options::setErrorOnUnmatchedNode);

  int (RunVerilator::*run)(const std::string&, const std::string&) const = &RunVerilator::run;

  class_<RunVerilator, boost::noncopyable>("RunVerilator",
                                           init<const std::string&>())
    .def("run", run);

  class_<Waypoints>("Waypoints")
    .def(init<const std::string, const std::string>())
    .def("add_start_point",      &Waypoints::addStartPoint)
    .def("add_finish_point",     &Waypoints::addEndPoint)
    .def("add_through_point",    &Waypoints::addThroughPoint)
    .def("add_avoid_point",      &Waypoints::addAvoidPoint);

  class_<Netlist, boost::noncopyable>("Netlist",
                                      init<const std::string&>())
    .def("get_named_vertices",     &Netlist::getNamedVerticesPtr,
                                   get_named_vertices_overloads())
    .def("get_reg_vertices",       &Netlist::getRegVerticesPtr,
                                   get_reg_vertices_overloads())
    .def("get_net_vertices",       &Netlist::getNetVerticesPtr,
                                   get_net_vertices_overloads())
    .def("get_port_vertices",      &Netlist::getPortVerticesPtr,
                                   get_port_vertices_overloads())
    .def("reg_exists",             &Netlist::regExists)
    .def("any_reg_exists",         &Netlist::anyRegExists)
    .def("startpoint_exists",      &Netlist::startpointExists)
    .def("endpoint_exists",        &Netlist::endpointExists)
    .def("any_startpoint_exists",  &Netlist::anyStartpointExists)
    .def("any_endpoint_exists",    &Netlist::anyEndpointExists)
    .def("path_exists",            &Netlist::pathExists)
    .def("get_any_path",           &Netlist::getAnyPath)
    .def("get_all_paths",          &Netlist::getAllPaths)
    .def("get_all_fanout_paths",   &Netlist::getAllFanOut)
    .def("get_all_fanin_paths",    &Netlist::getAllFanIn)
    .def("get_dtype_width",        &Netlist::getDTypeWidth)
    .def("get_named_dtypes",       &Netlist::getNamedDTypes)
    .def("get_vertex_dtype_str",   &Netlist::getVertexDTypeStr,
                                   get_vertex_dtype_str_overloads())
    .def("get_vertex_dtype_width", &Netlist::getVertexDTypeWidth,
                                   get_vertex_dtype_width_overloads())
    .def("dump_dot_file",          &Netlist::dumpDotFile);
}
