#include "cliutils.h"

#include <cstdlib>
#include <iostream>
#include <string>

void print_usage(const char *exe_name) {
  std::cout << "Usage: " << exe_name
            << " [--classes N] [--per-class N] [--attract yes|no]"
               " [--strength FLOAT] [--radius FLOAT] [--damping FLOAT]"
               " [--grid FLOAT] [--fpps N]\n"
            << "Defaults: --classes 1 --per-class 1000 --attract no --strength "
               "0.0 --radius 1.0 --damping 1.0 --grid (radius) --fpps 1\n"
            << "--fpps allowed values: 1, 2, 4, 8, 12\n";
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

static bool is_valid_fpps(size_t fpps) {
  return fpps == 1u || fpps == 2u || fpps == 4u || fpps == 8u || fpps == 12u;
}

bool parse_sim_config(int argc, char **argv, SimConfig &out_config,
                      bool &out_show_help) {
  out_show_help = false;
  bool grid_set = false;
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
    if (arg == "--radius") {
      if (i + 1 >= argc) {
        std::cerr << "--radius expects a value\n";
        return false;
      }
      if (!parse_float_arg(argv[++i], out_config.interaction_radius)) {
        std::cerr << "Invalid --radius value\n";
        return false;
      }
      continue;
    }
    if (arg == "--damping") {
      if (i + 1 >= argc) {
        std::cerr << "--damping expects a value\n";
        return false;
      }
      if (!parse_float_arg(argv[++i], out_config.damping)) {
        std::cerr << "Invalid --damping value\n";
        return false;
      }
      continue;
    }
    if (arg == "--grid") {
      if (i + 1 >= argc) {
        std::cerr << "--grid expects a value\n";
        return false;
      }
      if (!parse_float_arg(argv[++i], out_config.grid_cell_size)) {
        std::cerr << "Invalid --grid value\n";
        return false;
      }
      grid_set = true;
      continue;
    }
    if (arg == "--fpps") {
      if (i + 1 >= argc) {
        std::cerr << "--fpps expects a value\n";
        return false;
      }
      if (!parse_size_arg(argv[++i], out_config.fpps)) {
        std::cerr << "Invalid --fpps value\n";
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
  if (out_config.interaction_radius < 0.0f) {
    std::cerr << "--radius must be >= 0\n";
    return false;
  }
  if (out_config.damping < 0.0f) {
    std::cerr << "--damping must be >= 0\n";
    return false;
  }
  if (!grid_set) {
    out_config.grid_cell_size = out_config.interaction_radius;
  }
  if (out_config.grid_cell_size <= 0.0f) {
    std::cerr << "--grid must be > 0\n";
    return false;
  }
  if (out_config.fpps == 0) {
    std::cerr << "--fpps must be >= 1\n";
    return false;
  }
  if (!is_valid_fpps(out_config.fpps)) {
    std::cerr << "--fpps must be one of: 1, 2, 4, 8, 12\n";
    return false;
  }
  return true;
}
