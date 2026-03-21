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
