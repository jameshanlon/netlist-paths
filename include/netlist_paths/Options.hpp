#ifndef NETLIST_PATHS_OPTIONS_HPP
#define NETLIST_PATHS_OPTIONS_HPP

namespace netlist_paths {

constexpr const char *DEFAULT_OUTPUT_FILENAME = "netlist";

enum class MatchType {
  EXACT,
  REGEX,
  WILDCARD
};

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
  void setVerbose() { verboseMode = true; }
  void setDebug() { debugMode = true; }

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
      ignoreHierarchyMarkers(false) {}

public:
  // Prevent copies from being made (C++11).
  Options(Options const&) = delete;
  void operator=(Options const&) = delete;
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_OPTIONS_HPP
