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

#pragma once

#include <string>

namespace gm {
struct KeyBindings {
  char left = '\0';
  char right = '\0';
  char down = '\0';
  char rotate = '\0';
  char rotate_counterclockwise = '\0';
  char rotate_180 = '\0';
  char hard_drop = '\0';
  char hold = '\0';
  char pause = '\0';
  char restart = '\0';
  char quit = '\0';
};

void set_sound_enabled(bool enabled);
void set_visual_feedback_enabled(bool enabled);
bool consume_feedback_pulse();
void set_input_timing(int das_ms, int arr_ms);
void set_rotation_repeat_ms(int repeat_ms);
void set_input_repeat_ms(int repeat_ms);
void configure_keys(const KeyBindings& keys);
std::string read_key();
void handle_command(const std::string& command);
void process_pending_commands();
void queue_command_for_test(const std::string& command);
void key_event();
void start_listener();

// keyboard commands
void command_quit();
void command_rotate();
void command_rotate_counterclockwise();
void command_rotate_180();
void command_left();
void command_right();
void command_down();
void command_hard_drop();
void command_hold();
void command_pause();
void command_restart();
}  // namespace gm
