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

public:
  bool isMatchExact() const { return matchType == MatchType::EXACT; }
  bool isMatchRegex() const { return matchType == MatchType::REGEX; }
  bool isMatchWildcard() const { return matchType == MatchType::WILDCARD; }
  bool shouldIgnoreHierarchyMarkers() const { return ignoreHierarchyMarkers; }
  bool isVerboseMode() const { return verboseMode; }
  bool isDebugMode() const { return debugMode; }

  void setMatchWildcard() { matchType = MatchType::WILDCARD; }
  void setMatchRegex() { matchType = MatchType::REGEX; }
  void setMatchExact() { matchType = MatchType::EXACT; }
  void setIgnoreHierarchyMarkers() { ignoreHierarchyMarkers = true; }
  void setRespectHierarchyMarkers() { ignoreHierarchyMarkers = false; }
  void setVerbose() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
  }
  void setDebug() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
  }
  void setQuiet() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
  }

public:
  // Singleton instance.
  static Options &getInstance() {
    static Options instance;
    return instance;
  }
  static Options *getInstancePtr() {
    return &getInstance();
  }

private:
  Options() :
      debugMode(false),
      verboseMode(false),
      matchType(MatchType::EXACT),
      ignoreHierarchyMarkers(false) {
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
