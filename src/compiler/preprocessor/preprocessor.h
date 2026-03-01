/*
 * This defines preprocessor wrapper struct
 * AND
 * a wrapper for analysis outputs (NOT YET IMPLEMENTED)
 */

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Preprocessor {

  struct Output {
    bool success;
    std::string processed_source; // the new source file
    std::string error;
  };

  Preprocessor() : _include_dirs() {}

  Preprocessor(const std::vector<std::filesystem::path> &include_dirs)
      : _include_dirs(include_dirs) {}

  /*
   * Processes the file at file_path by expanding the includes
   *
   * if file is not a valid path, Output will contain :
   * {
   * success = false;
   * processed_source is EMPTY
   * error = "invalid path"
   * }
   */
  Output process_source(const std::filesystem::path &file_path,
                        bool verbose = false);

private:
  std::vector<std::filesystem::path>
      _include_dirs; // directories in which to look for includes

  std::unordered_map<std::filesystem::path, std::string>
      _include_files_cache; // cache for included files

  std::vector<std::filesystem::path>
      _expansion_stack; // stack to detect for included files

  // checks if file is in stack
  bool _is_file_in_stack(const std::filesystem::path &file);

  /*
   * generates the path to the file corresponding to an include of shape
   * #include "...."
   *
   * include_from: the path to the file where the #include has been read
   * include_string: the raw include string
   *
   * example:
   * #include "complex.glsl" has include_string "complex.glsl"
   *
   * returns an optional filepath with a value if and only if
   * the include_string corresponded to a correct path
   */
  std::optional<std::filesystem::path>
  _resolve_include(const std::filesystem::path &include_from,
                   const std::string &include_string);

  /*
   * private processing of the file expansions
   *
   * Recursively merge the file at <file_path> and its included files
   * into a single one.
   *
   * if file is not a valid path, Output will contain :
   * {
   * success = false;
   * processed_source is EMPTY
   * error = "invalid path"
   * }
   */
  Output _expand(const std::filesystem::path &file_path, bool verbose = false);
};

#endif
