#ifndef NETLIST_PATHS_UTILITIES_HPP
#define NETLIST_PATHS_UTILITIES_HPP

namespace netlist_paths {

/// Return true if pattern matches text.
///  * matches zero or more characters.
///  ? matches one character.
/// Algorithm from:
///   https://www.codeproject.com/Articles/188256/A-Simple-Wildcard-Matching-Function
bool wildcardMatch(const char *text, const char *pattern) {
  while (*pattern != '\0') {
    if (*pattern == '*') {
      // Ignore the * and check for matches against the remaining pattern and text.
      if (wildcardMatch(text, pattern+1)) {
        return true;
      }
      // Try matching the * against the next largest suffix of the text.
      if (wildcardMatch(text+1, pattern)) {
        return true;
      }
      // No matches.
      return false;
    } else if (*pattern == '?') {
      // Match any character, providing there is one, then advance pattern and text.
      if (*text == '\0') {
        return false;
      }
      pattern++;
      text++;
    } else {
      // Compare two characters, if they match advance the pattern and text.
      if (*pattern != *text) {
        return false;
      }
      pattern++;
      text++;
    }
  }
  // Match only if the pattern and text have ended.
  return *text == '\0' && *pattern == '\0';
}

bool wildcardMatch(const std::string &text, const std::string &pattern) {
  return wildcardMatch(text.c_str(), pattern.c_str());
}

} // End namespace.

#endif // NETLIST_PATHS_UTILITIES_HPP
