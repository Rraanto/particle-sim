/*
 * defines the Compiler class that wraps for glfw compiler
 */

#ifndef COMPILER_H
#define COMPILER_H

#include <filesystem>
#include <vector>
#include <string>
#include <glad/glad.h>

#include "preprocessor/preprocessor.h"

struct Compiler {
private:
  Preprocessor _preprocessor; // a preprocessor attached

public:
  struct CompileOutput {
    bool success;
    GLuint shader = 0; // the output shader if success
    std::string error;
  };

  // explicit avoids implicit cast to other types
  explicit Compiler() : _preprocessor() {}

  explicit Compiler(const std::vector<std::filesystem::path> &include_dirs)
      : _preprocessor(include_dirs) {}

  /*
   * Compiles the shader source to the openGL unsigned int out
   *
   * shader_type must be a valid OpenGL shader type
   *
   * in the use cases of this project: only two are used :
   * GL_FRAGMENT_SHADER, GL_VERTEX_SHADER
   */
  CompileOutput compile(const std::filesystem::path &source_file,
                        GLenum shader_type, bool verbose = false);
};

#endif
