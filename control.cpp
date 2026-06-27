// Copyright (c) 2024 Shaojie Li (shaojieli.nlp@gmail.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "./control.h"
#include "./define.h"
#include "./game.h"

#include <algorithm>

#ifdef _WIN32
#include <conio.h>
#endif

namespace gm {
namespace {
std::mutex input_mutex;
std::deque<std::string> pending_commands;
bool sound_enabled = false;
bool visual_feedback_enabled = false;
bool feedback_pulse = false;
int input_das_ms = 170;
int input_arr_ms = 35;
int rotation_repeat_ms = 120;
std::string last_processed_command;
std::chrono::steady_clock::time_point last_processed_at;
bool repeated_command_active = false;
}  // namespace

std::map<char, std::function<void()>> comm_func = {
    {KEY_Q, command_quit},      {KEY_W, command_rotate},
    {KEY_Z, command_rotate_counterclockwise},
    {KEY_X, command_rotate_180},
    {KEY_A, command_left},      {KEY_D, command_right},
    {KEY_S, command_down},      {KEY_SPACE, command_hard_drop},
    {KEY_C, command_hold},      {KEY_P, command_pause},
    {KEY_R, command_restart},   {'\003', command_quit},
    {'Q', command_quit},        {'W', command_rotate},
    {'Z', command_rotate_counterclockwise},
    {'X', command_rotate_180},
    {'A', command_left},        {'D', command_right},
    {'S', command_down},        {'C', command_hold},
    {'P', command_pause},       {'R', command_restart},
};

void bind_key(char key, std::function<void()> command) {
  if (key != '\0') {
    comm_func[key] = command;
  }
}

void beep() {
  if (sound_enabled) {
    std::cout << '\a' << std::flush;
  }
}

void run_action(bool changed) {
  if (changed) {
    beep();
    if (visual_feedback_enabled) {
      feedback_pulse = true;
    }
  }
}

void set_sound_enabled(bool enabled) {
  sound_enabled = enabled;
}

void set_visual_feedback_enabled(bool enabled) {
  visual_feedback_enabled = enabled;
  feedback_pulse = false;
}

bool consume_feedback_pulse() {
  const bool pulse = feedback_pulse;
  feedback_pulse = false;
  return pulse;
}

void reset_repeat_state() {
  last_processed_command.clear();
  last_processed_at = {};
  repeated_command_active = false;
}

void set_input_timing(int das_ms, int arr_ms) {
  input_das_ms = std::max(0, das_ms);
  input_arr_ms = std::max(0, arr_ms);
  reset_repeat_state();
}

void set_rotation_repeat_ms(int repeat_ms) {
  rotation_repeat_ms = std::max(0, repeat_ms);
  reset_repeat_state();
}

void set_input_repeat_ms(int repeat_ms) {
  input_das_ms = std::max(0, repeat_ms);
  input_arr_ms = std::max(0, repeat_ms);
  reset_repeat_state();
}

void configure_keys(const KeyBindings& keys) {
  bind_key(keys.left, command_left);
  bind_key(keys.right, command_right);
  bind_key(keys.down, command_down);
  bind_key(keys.rotate, command_rotate);
  bind_key(keys.rotate_counterclockwise, command_rotate_counterclockwise);
  bind_key(keys.rotate_180, command_rotate_180);
  bind_key(keys.hard_drop, command_hard_drop);
  bind_key(keys.hold, command_hold);
  bind_key(keys.pause, command_pause);
  bind_key(keys.restart, command_restart);
  bind_key(keys.quit, command_quit);
}

std::string read_key() {
  std::string key;
#ifdef _WIN32
  const int first = _getch();
  if (first == 0 || first == 224) {
    key.push_back('\033');
    key.push_back('[');
    switch (_getch()) {
      case 72:
        key.push_back('A');
        break;
      case 80:
        key.push_back('B');
        break;
      case 77:
        key.push_back('C');
        break;
      case 75:
        key.push_back('D');
        break;
      default:
        key.push_back('?');
        break;
    }
    return key;
  }
  key.push_back(static_cast<char>(first));
  return key;
#else
  struct termios old, cur;
  tcgetattr(0, &cur);
  old = cur;
  cfmakeraw(&cur);
  tcsetattr(0, 0, &cur);

  key.push_back(static_cast<char>(getchar()));
  if (key[0] == '\033') {
    key.push_back(static_cast<char>(getchar()));
    key.push_back(static_cast<char>(getchar()));
  }

  tcsetattr(0, 0, &old);
  return key;
#endif
}

void handle_escape_sequence(const std::string& command) {
  if (command.size() != 3 || command[1] != '[') {
    return;
  }

  switch (command[2]) {
    case 'A':
      command_rotate();
      break;
    case 'B':
      command_down();
      break;
    case 'C':
      command_right();
      break;
    case 'D':
      command_left();
      break;
    default:
      break;
  }
}

void key_event() {
  while (running.load()) {
    const std::string command = read_key();
    std::lock_guard<std::mutex> lock(input_mutex);
    pending_commands.push_back(command);
  }
}

void start_listener() {
  std::thread t(key_event);
  t.detach();
}

void command_quit() { quit(); }
void command_rotate() { run_action(rotate()); }
void command_rotate_counterclockwise() { run_action(rotate_counterclockwise()); }
void command_rotate_180() { run_action(rotate_180()); }
void command_left() { run_action(left()); }
void command_right() { run_action(right()); }
void command_down() { run_action(down()); }
void command_hard_drop() { run_action(hard_drop()); }
void command_hold() { run_action(hold()); }
void command_pause() { run_action(toggle_pause()); }
void command_restart() { run_action(restart()); }

void handle_command(const std::string& command) {
  if (command.empty()) {
    return;
  }
  if (command[0] == '\033') {
    handle_escape_sequence(command);
    return;
  }
  if (comm_func.find(command[0]) != comm_func.end()) {
    comm_func[command[0]]();
  }
}

bool is_rotation_command(const std::string& command) {
  if (command == "\033[A") {
    return true;
  }
  if (command.empty()) {
    return false;
  }
  return command[0] == KEY_W || command[0] == KEY_Z || command[0] == KEY_X ||
         command[0] == 'W' || command[0] == 'Z' || command[0] == 'X';
}

void process_pending_commands() {
  std::deque<std::string> commands;
  {
    std::lock_guard<std::mutex> lock(input_mutex);
    commands.swap(pending_commands);
  }
  for (const auto& command : commands) {
    const auto now = std::chrono::steady_clock::now();
    if (command == last_processed_command) {
      const int interval_ms =
          is_rotation_command(command)
              ? rotation_repeat_ms
              : (repeated_command_active ? input_arr_ms : input_das_ms);
      if (interval_ms > 0 &&
          now - last_processed_at < std::chrono::milliseconds(interval_ms)) {
        continue;
      }
      repeated_command_active = true;
    } else {
      repeated_command_active = false;
    }
    handle_command(command);
    last_processed_command = command;
    last_processed_at = now;
  }
}

void queue_command_for_test(const std::string& command) {
  std::lock_guard<std::mutex> lock(input_mutex);
  pending_commands.push_back(command);
}
}  // namespace gm
