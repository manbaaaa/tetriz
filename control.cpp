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

namespace gm {
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

std::string read_key() {
  std::string key;
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
    handle_command(command);
  }
}

void start_listener() {
  std::thread t(key_event);
  t.detach();
}

void command_quit() { quit(); }
void command_rotate() { rotate(); }
void command_rotate_counterclockwise() { rotate_counterclockwise(); }
void command_rotate_180() { rotate_180(); }
void command_left() { left(); }
void command_right() { right(); }
void command_down() { down(); }
void command_hard_drop() { hard_drop(); }
void command_hold() { hold(); }
void command_pause() { toggle_pause(); }
void command_restart() { restart(); }

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
}  // namespace gm
