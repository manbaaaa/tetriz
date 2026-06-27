#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

#include "config.h"

namespace {
void assert_config_file_loads_values() {
  const std::string path = "tetriz_test_config.rc";
  {
    std::ofstream file(path);
    file << "# tetriz config\n";
    file << "high_score_path = scores.dat\n";
    file << "start_level = 7\n";
    file << "hide_ghost = true\n";
    file << "hide_fps = yes\n";
    file << "sound = on\n";
    file << "visual_feedback = true\n";
    file << "input_das_ms = 120\n";
    file << "input_arr_ms = 20\n";
    file << "rotation_repeat_ms = 90\n";
    file << "key.left = h\n";
    file << "key.hard_drop = space\n";
  }

  const cfg::Config config = cfg::load_file(path);
  assert(config.high_score_path == "scores.dat");
  assert(config.start_level == 7);
  assert(config.hide_ghost);
  assert(config.hide_fps);
  assert(config.sound);
  assert(config.visual_feedback);
  assert(config.input_das_ms == 120);
  assert(config.input_arr_ms == 20);
  assert(config.rotation_repeat_ms == 90);
  assert(config.keys.left == 'h');
  assert(config.keys.hard_drop == ' ');
  std::remove(path.c_str());
}

void assert_missing_config_uses_defaults() {
  const cfg::Config config = cfg::load_file("missing_tetriz_config.rc");
  assert(config.high_score_path == ".tetriz_high_score");
  assert(config.start_level == 1);
  assert(!config.hide_ghost);
  assert(!config.hide_fps);
  assert(!config.sound);
  assert(!config.visual_feedback);
  assert(config.input_das_ms == 170);
  assert(config.input_arr_ms == 35);
  assert(config.rotation_repeat_ms == 120);
}

void assert_legacy_repeat_config_sets_das_and_arr() {
  const std::string path = "tetriz_test_legacy_repeat.rc";
  {
    std::ofstream file(path);
    file << "input_repeat_ms = 80\n";
  }

  const cfg::Config config = cfg::load_file(path);
  assert(config.input_das_ms == 80);
  assert(config.input_arr_ms == 80);
  std::remove(path.c_str());
}
}  // namespace

int main() {
  assert_config_file_loads_values();
  assert_missing_config_uses_defaults();
  assert_legacy_repeat_config_sets_das_and_arr();
  return 0;
}
