#ifndef NETLIST_PATHS_OPTIONS_HPP
#define NETLIST_PATHS_OPTIONS_HPP

namespace netlist_paths {

constexpr const char *DEFAULT_OUTPUT_FILENAME = "netlist";

struct Options {
  bool debugMode;
  bool verboseMode;
  bool displayHelp;
  bool dumpDotfile;
  bool dumpNames;
  bool allPaths;
  bool startPoints;
  bool endPoints;
  bool fanOutDegree;
  bool fanInDegree;
  bool reportLogic;
  bool fullFileNames;
  bool compile;
  bool boostParser;
  bool matchWildcard;
  bool getMatchWildcard() const { return matchWildcard; }
  void setMatchWildcard() { matchWildcard = true; }
  void setMatchRegex() { matchWildcard = false; }
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
      displayHelp(false),
      dumpDotfile(false),
      dumpNames(false),
      allPaths(false),
      startPoints(false),
      endPoints(false),
      fanOutDegree(false),
      fanInDegree(false),
      reportLogic(false),
      fullFileNames(false),
      compile(false),
      boostParser(false),
      matchWildcard(true) {}
public:
  // Prevent copies from being made (C++11).
  Options(Options const&) = delete;
  void operator=(Options const&) = delete;
};

} // End netlist_paths namespace.

#endif // NETLIST_PATHS_OPTIONS_HPP
