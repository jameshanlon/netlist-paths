#ifndef NETLIST_PATHS_LOCATION_HPP
#define NETLIST_PATHS_LOCATION_HPP

#include <string>
#include <memory>
#include <boost/format.hpp>

class File {
  std::string filename;
  std::string language;
public:
  File(const std::string &filename,
       const std::string &language) :
      filename(filename), language(language) {}
  const std::string &getFilename() const { return filename; }
};

class Location {
  std::shared_ptr<File> file;
  unsigned startLine;
  unsigned startCol;
  unsigned endLine;
  unsigned endCol;
public:
  Location() : file(nullptr) {}
  Location(std::shared_ptr<File> file,
           unsigned startLine,
           unsigned startCol,
           unsigned endLine,
           unsigned endCol) :
      file(file),
      startLine(startLine),
      startCol(startCol),
      endLine(endLine),
      endCol(endCol) {}
  const std::string getFilename() const { return file->getFilename(); }
  /// Equality comparison
  friend bool operator== (const Location &a, const Location &b) {
    return a.file == b.file &&
           a.startLine == b.startLine &&
           a.startCol == b.startCol &&
           a.endLine == b.endLine &&
           a.endCol == b.endCol;
  }
  std::string getLocationStrExact() const {
    auto s = boost::format("%s %d:%d,%d:%d")
               % getFilename() % startLine % startCol % endLine % endCol;
    return s.str();
  }
  std::string getLocationStr() const {
    auto s = boost::format("%s:%d") % getFilename() % startLine;
    return s.str();
  }
};

#endif // NETLIST_PATHS_LOCATION_HPP
