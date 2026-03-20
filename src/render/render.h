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

struct RenderParticle {
  float x = 0.0f;
  float y = 0.0f;
  int class_id = 0;
};

class ParticleRenderer {
private:
  GLuint _vertex_shader;
  GLuint _fragment_shader;
  GLuint _shader_program;

  // vertex objects
  GLuint _vbo;
  GLuint _vao;

  int _max_particles = 100; // max amount of particles

  GLint _u_center = -1;
  GLint _u_scale = -1;
  GLint _u_zoom = -1;
  GLint _u_aspect = -1;
  GLint _u_time = -1;

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
  void draw(size_t count, const Camera &camera, float aspect, float time_sec);

  /*
   * shutdown
   */
  void shutdown();
};

#endif
