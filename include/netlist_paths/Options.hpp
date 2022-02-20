#ifndef NETLIST_PATHS_OPTIONS_HPP
#define NETLIST_PATHS_OPTIONS_HPP

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace netlist_paths {

constexpr const char *DEFAULT_OUTPUT_FILENAME = "netlist";

enum class MatchType {
  EXACT,
  REGEX,
  WILDCARD
};

/// A class encapsulating options.
class Options {

  bool debugMode;
  bool verboseMode;
  MatchType matchType;
  bool ignoreHierarchyMarkers;
  bool matchOneVertex;
  bool traverseRegisters;
  bool restrictStartPoints;
  bool restrictEndPoints;
  bool errorOnUnmatchedNode;

public:
  bool isMatchExact() const { return matchType == MatchType::EXACT; }
  bool isMatchRegex() const { return matchType == MatchType::REGEX; }
  bool isMatchWildcard() const { return matchType == MatchType::WILDCARD; }
  bool isMatchOneVertex() const { return matchOneVertex; }
  bool isMatchAnyVertex() const { return !matchOneVertex; }
  bool shouldIgnoreHierarchyMarkers() const { return ignoreHierarchyMarkers; }
  bool shouldTraverseRegisters() const { return traverseRegisters; }
  bool isRestrictStartPoints() const { return restrictStartPoints; }
  bool isRestrictEndPoints() const { return restrictEndPoints; }
  bool isVerboseMode() const { return verboseMode; }
  bool isDebugMode() const { return debugMode; }
  bool isErrorOnUnmatchedNode() const { return errorOnUnmatchedNode; }

  /// Set matching to use wildcards.
  void setMatchWildcard() { matchType = MatchType::WILDCARD; }

  /// Set matching to use regular expressions.
  void setMatchRegex() { matchType = MatchType::REGEX; }

  /// Set matching to be exact.
  void setMatchExact() { matchType = MatchType::EXACT; }

  /// Set matching to ignore (true) or respect (false) hierarchy markers (only
  /// with wildcard or regular expression matching modes). Note that hierarchy
  /// markers are '.', '/' and '_'.
  void setIgnoreHierarchyMarkers(bool value) { ignoreHierarchyMarkers = value; }

  /// Set matching to identify one vertex, and for it to be an error if more
  /// than one vertex is matched.
  void setMatchOneVertex() { matchOneVertex = true; }

  /// Set matching to identify multiple vertices, and for just one to be
  /// chosen arbitrarily.
  void setMatchAnyVertex() { matchOneVertex = false; }

  /// Enable or disable path traversal of registers.
  void setTraverseRegisters(bool value) { traverseRegisters = value; }

  /// Set path start point restriction. When set to true, paths must start on
  /// top-level ports or registers. When set to false, paths can start on any
  /// variable.
  void setRestrictStartPoints(bool value) { restrictStartPoints = value; }

  /// Set path end point restriction. When set to true, paths must end on
  /// top-level ports or registers. When set to false, paths can end on any
  /// variable.
  void setRestrictEndPoints(bool value) { restrictEndPoints = value; }

  /// Setup the XML parser to raise an error when an unmatched node is
  /// encountered. For testing purposes only.
  void setErrorOnUnmatchedNode(bool value) { errorOnUnmatchedNode = value; }

  /// Enable verbose output.
  void setLoggingVerbose() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
  }

  /// Enable debug output (including verbose messages).
  void setLoggingDebug() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
  }

  /// Supress verbose and debug messages.
  void setLoggingQuiet() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
  }

  /// Supress warning, verbose and debug messages.
  void setLoggingErrorOnly() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::error);
  }

public:

  /// Return a reference to the Options object.
  static Options &getInstance() {
    static Options instance;
    return instance;
  }

  /// Return a pointer to the Options object.
  static Options *getInstancePtr() {
    return &getInstance();
  }

private:
  Options() :
      debugMode(false),
      verboseMode(false),
      matchType(MatchType::EXACT),
      ignoreHierarchyMarkers(false),
      matchOneVertex(true),
      traverseRegisters(false),
      restrictStartPoints(true),
      restrictEndPoints(true),
      errorOnUnmatchedNode(false) {
    // Setup logging.
    boost::log::add_console_log(std::clog, boost::log::keywords::format = "%Severity%: %Message%");
    setLoggingQuiet();
  }

public:
  // Prevent copies from being made (C++11).
  Options(Options const&) = delete;
  void operator=(Options const&) = delete;
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_OPTIONS_HPP
