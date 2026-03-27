#ifndef CLIUTILS_H
#define CLIUTILS_H

/*
 * This is a small CLI utility parser library
 */

#include <cstddef>

struct SimConfig {
  /*
   * Configures a simulation
   */
  size_t class_count = 1;
  size_t particles_per_class = 1000;
  bool attraction_enabled = false;
  float attraction_strength = 1.0f;
  float interaction_radius = 1.0f;
  float damping = 1.0f;
  float grid_cell_size = 0.2f;
  size_t fpps = 1;
};

/*
 * shows help
 */
void print_usage(const char *exe_name);

/*
 * parses a command line argument into a sim config
 */
bool parse_sim_config(int argc, char **argv, SimConfig &out_config,
                      bool &out_show_help);

#endif
