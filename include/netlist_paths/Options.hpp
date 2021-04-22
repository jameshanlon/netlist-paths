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

public:
  bool isMatchExact() const { return matchType == MatchType::EXACT; }
  bool isMatchRegex() const { return matchType == MatchType::REGEX; }
  bool isMatchWildcard() const { return matchType == MatchType::WILDCARD; }
  bool isMatchOneVertex() const { return matchOneVertex; }
  bool isMatchAnyVertex() const { return !matchOneVertex; }
  bool shouldIgnoreHierarchyMarkers() const { return ignoreHierarchyMarkers; }
  bool isVerboseMode() const { return verboseMode; }
  bool isDebugMode() const { return debugMode; }

  /// Set matching to use wildcards.
  void setMatchWildcard() { matchType = MatchType::WILDCARD; }

  /// Set matching to use regular expressions.
  void setMatchRegex() { matchType = MatchType::REGEX; }

  /// Set matching to be exact.
  void setMatchExact() { matchType = MatchType::EXACT; }

  /// Set matching to ignore hierarchy markers (only with wildcard or regular
  /// expression matching modes). Hierarchy markers are '.', '/' and '_'.
  void setIgnoreHierarchyMarkers() { ignoreHierarchyMarkers = true; }

  /// Set matching to respect hierarchy markers (ie not to ignore them, only
  /// with wildcard or regular expression matching modes).
  void setRespectHierarchyMarkers() { ignoreHierarchyMarkers = false; }

  /// Set matching to identify one vertex, and for it to be an error if more
  /// than one vertex is matched.
  void setMatchOneVertex() { matchOneVertex = true; }

  /// Set matching to identify multiple vertices, and for just one to be
  /// chosen arbitrarily.
  void setMatchAnyVertex() { matchOneVertex = false; }

  /// Enable verbose output.
  void setVerbose() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
  }

  /// Enable debug output (including verbose messages).
  void setDebug() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
  }

  /// Supress verbose and debug messages.
  void setQuiet() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
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
      matchOneVertex(true) {
    // Setup logging.
    boost::log::add_console_log(std::clog, boost::log::keywords::format = "%Severity%: %Message%");
    setQuiet();
  }

public:
  // Prevent copies from being made (C++11).
  Options(Options const&) = delete;
  void operator=(Options const&) = delete;
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_OPTIONS_HPP
