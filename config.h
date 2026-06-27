#pragma once

#include <string>

namespace cfg {
struct KeyBindings {
  char left = '\0';
  char right = '\0';
  char down = '\0';
  char rotate = '\0';
  char rotate_ccw = '\0';
  char rotate_180 = '\0';
  char hard_drop = '\0';
  char hold = '\0';
  char pause = '\0';
  char restart = '\0';
  char quit = '\0';
};

struct Config {
  std::string high_score_path = ".tetriz_high_score";
  int start_level = 1;
  bool hide_ghost = false;
  bool hide_fps = false;
  bool sound = false;
  bool visual_feedback = false;
  int input_das_ms = 170;
  int input_arr_ms = 35;
  int rotation_repeat_ms = 120;
  KeyBindings keys;
};

Config load_file(const std::string& path);
void apply_env_overrides(Config* config);
}  // namespace cfg
