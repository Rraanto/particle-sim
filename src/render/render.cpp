#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "compiler.h"
#include "render.h"

bool ParticleRenderer::init(const std::filesystem::path &vertex_shader_path,
                            const std::filesystem::path &fragment_shader_path) {

  // compile shaders
  Compiler compiler;
  Compiler::CompileOutput compile_output;

  compile_output = compiler.compile(vertex_shader_path, GL_VERTEX_SHADER);
  if (!compile_output.success) {
    std::cerr << compile_output.error << std::endl;
    return false;
  }

  this->_vertex_shader = compile_output.shader;

  compile_output =
      compiler.compile(fragment_shader_path, GL_FRAGMENT_SHADER, true);
  if (!compile_output.success) {
    std::cerr << compile_output.error << std::endl;
    return false;
  }

  this->_fragment_shader = compile_output.shader;

  // attach shaders to a program
  this->_shader_program = glCreateProgram();

  glAttachShader(this->_shader_program, this->_vertex_shader);
  glAttachShader(this->_shader_program, this->_fragment_shader);
  glLinkProgram(this->_shader_program);

  int success = 0;
  char info_log[512] = {0};
  glGetProgramiv(this->_shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(this->_shader_program, 512, nullptr, info_log);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << "\n";
    glDeleteShader(this->_vertex_shader);
    glDeleteShader(this->_fragment_shader);
    glDeleteProgram(this->_shader_program);
    return false;
  }

  glDeleteShader(this->_vertex_shader);
  glDeleteShader(this->_fragment_shader);

  glUseProgram(this->_shader_program);

  // VAO and VBO setup
  unsigned int VBO;
  glGenBuffers(1, &VBO);

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // copy vertices array to those memory
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  return true;
}
