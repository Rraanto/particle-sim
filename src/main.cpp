#include <iostream>

#include "app.h"
#include "cliutils/cliutils.h"

int main(int argc, char **argv) {
  SimConfig sim_config;
  bool show_help = false;
  if (!parse_sim_config(argc, argv, sim_config, show_help)) {
    print_usage(argv[0]);
    return show_help ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  App app;
  if (!app.init(sim_config)) {
    return EXIT_FAILURE;
  }

  app.run();
  app.shutdown();
  return EXIT_SUCCESS;
}
