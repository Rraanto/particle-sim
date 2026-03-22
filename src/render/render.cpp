#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstddef>
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

  // associate shader program
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

  this->_u_center = glGetUniformLocation(this->_shader_program, "uCenter");
  this->_u_scale = glGetUniformLocation(this->_shader_program, "uScale");
  this->_u_zoom = glGetUniformLocation(this->_shader_program, "uZoom");
  this->_u_aspect = glGetUniformLocation(this->_shader_program, "uAspect");
  this->_u_time = glGetUniformLocation(this->_shader_program, "uTime");

  glEnable(GL_PROGRAM_POINT_SIZE);

  // VAO and VBO setup
  this->_vao = 0;
  glGenVertexArrays(1, &this->_vao);
  glBindVertexArray(this->_vao);

  this->_vbo = 0;
  glGenBuffers(1, &this->_vbo);

  // copy vertices array to those memory
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);

  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(
                   static_cast<long unsigned int>(this->_max_particles) *
                   sizeof(RenderParticle)),
               nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderParticle),
                        (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribIPointer(1, 1, GL_INT, sizeof(RenderParticle),
                         (void *)offsetof(RenderParticle, class_id));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(RenderParticle),
                        (void *)offsetof(RenderParticle, radius));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return true;
}

void ParticleRenderer::upload_particles(const std::vector<float> &pos_x,
                                        const std::vector<float> &pos_y,
                                        const std::vector<int> &classes,
                                        const std::vector<float> &class_masses) {

  // count amount of particle classes, abort if none
  size_t count = std::min(pos_x.size(), pos_y.size());
  if (!classes.empty()) {
    count = std::min(count, classes.size());
  }
  if (count == 0) {
    return;
  }

  // check if max amount of particles not reached
  if (count > static_cast<size_t>(this->_max_particles)) {
    this->_max_particles = static_cast<int>(count);
    glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(
                     static_cast<long unsigned int>(this->_max_particles) *
                     sizeof(RenderParticle)),
                 nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  std::vector<RenderParticle> particles;
  particles.reserve(count);
  constexpr float kBaseRadius = 2.0f;
  for (size_t i = 0; i < count; ++i) {
    RenderParticle p;
    p.x = pos_x[i];
    p.y = pos_y[i];
    float mass = 1.0f;
    if (i < classes.size()) {
      p.class_id = classes[i];
      const int class_id = classes[i];
      if (class_id >= 0 &&
          static_cast<size_t>(class_id) < class_masses.size()) {
        mass = class_masses[static_cast<size_t>(class_id)];
      }
    }
    if (mass < 0.0f) {
      mass = 0.0f;
    }
    p.radius = std::max(0.5f, mass * kBaseRadius);
    particles.push_back(p);
  }

  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferSubData(
      GL_ARRAY_BUFFER, 0,
      static_cast<GLsizeiptr>(particles.size() * sizeof(RenderParticle)),
      particles.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleRenderer::draw(size_t count, const Camera &camera, float aspect,
                            float time_sec) {
  if (count == 0) {
    return;
  }

  glUseProgram(this->_shader_program);

  if (this->_u_center >= 0) {
    glUniform2f(this->_u_center, camera.get_x(), camera.get_y());
  }
  if (this->_u_scale >= 0) {
    glUniform1f(this->_u_scale, camera.get_scale());
  }
  if (this->_u_zoom >= 0) {
    glUniform1f(this->_u_zoom, camera.get_zoom());
  }
  if (this->_u_aspect >= 0) {
    glUniform1f(this->_u_aspect, aspect);
  }
  if (this->_u_time >= 0) {
    glUniform1f(this->_u_time, time_sec);
  }

  glBindVertexArray(this->_vao);

  // draw the <count> first particles
  glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count));
  glBindVertexArray(0);
}

void ParticleRenderer::shutdown() {
  if (this->_vbo != 0u) {
    glDeleteBuffers(1, &this->_vbo);
    this->_vbo = 0;
  }

  if (this->_vao != 0u) {
    glDeleteVertexArrays(1, &this->_vao);
    this->_vao = 0;
  }

  if (this->_shader_program != 0u) {
    glDeleteProgram(this->_shader_program);
    this->_shader_program = 0;
  }
}
