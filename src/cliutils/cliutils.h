#ifndef CLIUTILS_H
#define CLIUTILS_H

/*
 * This is a small CLI utility parser library
 */

#include "config/app_config.h"

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
bool parse_sim_config(int argc, char **argv, AppConfig &out_config,
                      bool &out_show_help);

#endif
