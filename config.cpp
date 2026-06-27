#include "./config.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <string>

namespace cfg {
namespace {
std::string trim(const std::string& value) {
  const auto begin = value.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return "";
  }
  const auto end = value.find_last_not_of(" \t\r\n");
  return value.substr(begin, end - begin + 1);
}

std::string lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

bool parse_bool(const std::string& value, bool fallback) {
  const std::string normalized = lower(trim(value));
  if (normalized == "1" || normalized == "true" || normalized == "yes" ||
      normalized == "on") {
    return true;
  }
  if (normalized == "0" || normalized == "false" || normalized == "no" ||
      normalized == "off") {
    return false;
  }
  return fallback;
}

int parse_int(const std::string& value, int fallback) {
  try {
    return std::stoi(trim(value));
  } catch (...) {
    return fallback;
  }
}

char parse_key(const std::string& value) {
  const std::string normalized = trim(value);
  if (normalized == "space") {
    return ' ';
  }
  if (normalized == "tab") {
    return '\t';
  }
  if (normalized.empty()) {
    return '\0';
  }
  return normalized[0];
}

void apply_value(const std::string& key, const std::string& value, Config* config) {
  if (key == "high_score_path") {
    config->high_score_path = value;
  } else if (key == "start_level") {
    config->start_level = parse_int(value, config->start_level);
  } else if (key == "hide_ghost") {
    config->hide_ghost = parse_bool(value, config->hide_ghost);
  } else if (key == "hide_fps") {
    config->hide_fps = parse_bool(value, config->hide_fps);
  } else if (key == "sound") {
    config->sound = parse_bool(value, config->sound);
  } else if (key == "visual_feedback") {
    config->visual_feedback = parse_bool(value, config->visual_feedback);
  } else if (key == "input_das_ms") {
    config->input_das_ms = parse_int(value, config->input_das_ms);
  } else if (key == "input_arr_ms") {
    config->input_arr_ms = parse_int(value, config->input_arr_ms);
  } else if (key == "rotation_repeat_ms") {
    config->rotation_repeat_ms =
        parse_int(value, config->rotation_repeat_ms);
  } else if (key == "input_repeat_ms") {
    const int repeat_ms = parse_int(value, config->input_arr_ms);
    config->input_das_ms = repeat_ms;
    config->input_arr_ms = repeat_ms;
  } else if (key == "key.left") {
    config->keys.left = parse_key(value);
  } else if (key == "key.right") {
    config->keys.right = parse_key(value);
  } else if (key == "key.down") {
    config->keys.down = parse_key(value);
  } else if (key == "key.rotate") {
    config->keys.rotate = parse_key(value);
  } else if (key == "key.rotate_ccw") {
    config->keys.rotate_ccw = parse_key(value);
  } else if (key == "key.rotate_180") {
    config->keys.rotate_180 = parse_key(value);
  } else if (key == "key.hard_drop") {
    config->keys.hard_drop = parse_key(value);
  } else if (key == "key.hold") {
    config->keys.hold = parse_key(value);
  } else if (key == "key.pause") {
    config->keys.pause = parse_key(value);
  } else if (key == "key.restart") {
    config->keys.restart = parse_key(value);
  } else if (key == "key.quit") {
    config->keys.quit = parse_key(value);
  }
}
}  // namespace

Config load_file(const std::string& path) {
  Config config;
  std::ifstream file(path);
  std::string line;
  while (std::getline(file, line)) {
    const auto comment = line.find('#');
    if (comment != std::string::npos) {
      line = line.substr(0, comment);
    }
    const auto equals = line.find('=');
    if (equals == std::string::npos) {
      continue;
    }
    const std::string key = trim(line.substr(0, equals));
    const std::string value = trim(line.substr(equals + 1));
    apply_value(key, value, &config);
  }
  return config;
}

void apply_env_overrides(Config* config) {
  if (const char* high_score_path = std::getenv("TETRIZ_HIGH_SCORE_PATH")) {
    config->high_score_path = high_score_path;
  }
  if (const char* start_level = std::getenv("TETRIZ_START_LEVEL")) {
    config->start_level = parse_int(start_level, config->start_level);
  }
  if (const char* hide_ghost = std::getenv("TETRIZ_HIDE_GHOST")) {
    config->hide_ghost = parse_bool(hide_ghost, true);
  }
  if (const char* hide_fps = std::getenv("TETRIZ_HIDE_FPS")) {
    config->hide_fps = parse_bool(hide_fps, true);
  }
  if (const char* sound = std::getenv("TETRIZ_SOUND")) {
    config->sound = parse_bool(sound, config->sound);
  }
  if (const char* visual_feedback = std::getenv("TETRIZ_VISUAL_FEEDBACK")) {
    config->visual_feedback =
        parse_bool(visual_feedback, config->visual_feedback);
  }
  if (const char* input_repeat_ms = std::getenv("TETRIZ_INPUT_REPEAT_MS")) {
    const int repeat_ms = parse_int(input_repeat_ms, config->input_arr_ms);
    config->input_das_ms = repeat_ms;
    config->input_arr_ms = repeat_ms;
  }
  if (const char* input_das_ms = std::getenv("TETRIZ_INPUT_DAS_MS")) {
    config->input_das_ms = parse_int(input_das_ms, config->input_das_ms);
  }
  if (const char* input_arr_ms = std::getenv("TETRIZ_INPUT_ARR_MS")) {
    config->input_arr_ms = parse_int(input_arr_ms, config->input_arr_ms);
  }
  if (const char* rotation_repeat_ms = std::getenv("TETRIZ_ROTATION_REPEAT_MS")) {
    config->rotation_repeat_ms =
        parse_int(rotation_repeat_ms, config->rotation_repeat_ms);
  }
}
}  // namespace cfg
