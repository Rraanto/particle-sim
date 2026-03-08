/*
 * This library allows the rendering of a simulation state
 * to the screen
 */

#ifndef RENDER_H
#define RENDER_H

#include <string>
#include <filesystem>

#include <glad/glad.h>
#include <vector>

#include "camera.h"

class ParticleRenderer {
private:
  GLuint _vertex_shader;
  GLuint _fragment_shader;
  GLuint _shader_program;

  // vertex objects
  unsigned int _vbo;
  unsigned int _vao;

public:
  /*
   * Initialises shader program:
   * Loads source, compiles the source, creates Vertex array object and vertex
   * buffer objects
   *
   * Assumes an existing openGL context
   *
   * returns false if initialisation failed
   */
  bool init(const std::filesystem::path &vertex_shader_path,
            const std::filesystem::path &fragment_shader_path);

  /*
   * Uploads particle information to the GPU
   */
  void upload_particles(const std::vector<float> &pos_x,
                        const std::vector<float> &pos_y,
                        std::vector<int> classes);

  /*
   * draws the specified amount of particles
   */
  void draw(size_t count);

  /*
   * shutdown
   */
  void shutdown();
};

#endif
