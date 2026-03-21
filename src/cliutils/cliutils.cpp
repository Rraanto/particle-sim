#include "cliutils.h"

#include <cstdlib>
#include <iostream>
#include <string>

void print_usage(const char *exe_name) {
  std::cout << "Usage: " << exe_name
            << " [--classes N] [--per-class N] [--attract yes|no] [--strength "
               "FLOAT]\n"
            << "Defaults: --classes 1 --per-class 1000 --attract no --strength "
               "1.0\n";
}

static bool parse_size_arg(const char *value, size_t &out_value) {
  if (!value) {
    return false;
  }
  char *end = nullptr;
  const unsigned long long parsed = std::strtoull(value, &end, 10);
  if (end == value || *end != '\0') {
    return false;
  }
  out_value = static_cast<size_t>(parsed);
  return true;
}

static bool parse_float_arg(const char *value, float &out_value) {
  if (!value) {
    return false;
  }
  char *end = nullptr;
  const float parsed = std::strtof(value, &end);
  if (end == value || *end != '\0') {
    return false;
  }
  out_value = parsed;
  return true;
}

static bool parse_bool_arg(const std::string &value, bool &out_value) {
  if (value == "1" || value == "true" || value == "yes" || value == "on") {
    out_value = true;
    return true;
  }
  if (value == "0" || value == "false" || value == "no" || value == "off") {
    out_value = false;
    return true;
  }
  return false;
}

bool parse_sim_config(int argc, char **argv, SimConfig &out_config,
                      bool &out_show_help) {
  out_show_help = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      out_show_help = true;
      return false;
    }
    if (arg == "--classes") {
      if (i + 1 >= argc) {
        std::cerr << "--classes expects a value\n";
        return false;
      }
      if (!parse_size_arg(argv[++i], out_config.class_count)) {
        std::cerr << "Invalid --classes value\n";
        return false;
      }
      continue;
    }
    if (arg == "--per-class") {
      if (i + 1 >= argc) {
        std::cerr << "--per-class expects a value\n";
        return false;
      }
      if (!parse_size_arg(argv[++i], out_config.particles_per_class)) {
        std::cerr << "Invalid --per-class value\n";
        return false;
      }
      continue;
    }
    if (arg == "--attract") {
      if (i + 1 >= argc) {
        std::cerr << "--attract expects yes|no\n";
        return false;
      }
      if (!parse_bool_arg(argv[++i], out_config.attraction_enabled)) {
        std::cerr << "Invalid --attract value (use yes|no)\n";
        return false;
      }
      continue;
    }
    if (arg == "--strength") {
      if (i + 1 >= argc) {
        std::cerr << "--strength expects a value\n";
        return false;
      }
      if (!parse_float_arg(argv[++i], out_config.attraction_strength)) {
        std::cerr << "Invalid --strength value\n";
        return false;
      }
      continue;
    }
    std::cerr << "Unknown argument: " << arg << "\n";
    return false;
  }

  if (out_config.class_count == 0) {
    std::cerr << "--classes must be >= 1\n";
    return false;
  }
  if (out_config.particles_per_class == 0) {
    std::cerr << "--per-class must be >= 1\n";
    return false;
  }
  return true;
}
