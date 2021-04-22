#ifndef NETLIST_PATHS_LOCATION_HPP
#define NETLIST_PATHS_LOCATION_HPP

#include <string>
#include <memory>
#include <boost/format.hpp>

/// A class representing a source file.
class File {
  std::string filename;
  std::string language;

public:

  /// Construct a source file object.
  ///
  /// \param filename The file name.
  /// \param language The source language of the file.
  File(const std::string &filename,
       const std::string &language) :
      filename(filename), language(language) {}

  const std::string &getFilename() const { return filename; }
  const std::string &getLanguage() const { return language; }
};

/// A class representing a file location.
class Location {
  std::shared_ptr<File> file;
  unsigned startLine;
  unsigned startCol;
  unsigned endLine;
  unsigned endCol;

public:

  /// Default construct a file location object.
  Location() : file(nullptr) {}

  /// Construct a file location object, identifying a source-level entity.
  ///
  /// \param file      A shared pointer to the file which this location is in.
  /// \param startLine The line number of the start of the entity.
  /// \param startCol  The column number of the start of the entity.
  /// \param endLine   The line number of the end of the entity.
  /// \param endCol    The column number of the end of the entity.
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

  /// Return the filename.
  const std::string getFilename() const {
    if (file) {
      return file->getFilename();
    } else {
      return "unknown";
    }
  }

  /// Equality comparison
  friend bool operator== (const Location &a, const Location &b) {
    return a.file == b.file &&
           a.startLine == b.startLine &&
           a.startCol == b.startCol &&
           a.endLine == b.endLine &&
           a.endCol == b.endCol;
  }

  /// Return a string representing the exact location (using all location details).
  std::string getLocationStrExact() const {
    auto s = boost::format("%s %d:%d,%d:%d")
               % getFilename() % startLine % startCol % endLine % endCol;
    return s.str();
  }

  /// Return a string representing a brief location (only filename and start line).
  std::string getLocationStr() const {
    auto s = boost::format("%s:%d") % getFilename() % startLine;
    return s.str();
  }
};

#endif // NETLIST_PATHS_LOCATION_HPP
