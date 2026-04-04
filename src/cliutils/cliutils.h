#ifndef CLIUTILS_H
#define CLIUTILS_H

/*
 * This is a small CLI utility parser library
 */

#include <cstddef>

/*
 * SimConfig stores the validated startup configuration used to build the app.
 */
struct SimConfig {
  /*
   * Particle-class configuration.
   */
  size_t class_count = 1;
  size_t particles_per_class = 1000;

  /*
   * Global force and integration controls.
   */
  bool attraction_enabled = false;
  float attraction_strength = 0.0f;
  float interaction_radius = 1.0f;
  float damping = 1.0f;
  float grid_cell_size = 0.2f;

  /*
   * Frames-per-physics-step selector.
   * Only the discrete values accepted by parse_sim_config() are valid.
   */
  size_t fpps = 1;
};

/*
 * Prints CLI usage and defaults to stdout.
 * exe_name is used as the displayed executable name.
 */
void print_usage(const char *exe_name);

/*
 * Parses argv into out_config and validates all supported options.
 * Returns true on success.
 * Returns false on parse/validation failure or when help was requested.
 * out_show_help is set to true only for --help / -h.
 */
bool parse_sim_config(int argc, char **argv, SimConfig &out_config,
                      bool &out_show_help);

#endif
