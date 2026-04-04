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
#include "params.h"

/*
 * RenderParticle is the packed GPU-side payload for one particle instance.
 */
struct RenderParticle {
  float x = 0.0f;
  float y = 0.0f;
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  int class_id = 0;
  float radius = 1.0f;
};

/*
 * ParticleRenderer owns the OpenGL objects required to draw particles.
 *
 * Contract:
 * - init() requires a valid current OpenGL context.
 * - upload_particles() refreshes the GPU buffer for the next draw().
 * - shutdown() releases owned GL objects and is safe after partial init.
 */
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
   * Compiles shaders and creates all GL objects needed for rendering.
   * Returns false when shader compilation/linking or buffer setup fails.
   */
  bool init(const std::filesystem::path &vertex_shader_path,
            const std::filesystem::path &fragment_shader_path);

  /*
   * Uploads the current particle snapshot to the GPU.
   * The input arrays are expected to describe the same particle ordering.
   */
  void upload_particles(const std::vector<float> &pos_x,
                        const std::vector<float> &pos_y,
                        const std::vector<int> &classes,
                        const std::vector<ClassParams> &class_params);

  /*
   * Draws the first count particles using the provided camera and timing state.
   * count may be smaller than the number of uploaded particles.
   */
  void draw(size_t count, const Camera &camera, float aspect, float time_sec);

  /*
   * Releases all owned OpenGL resources.
   * Safe to call even when initialization was incomplete.
   */
  void shutdown();
};

#endif
