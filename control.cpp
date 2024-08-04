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
char command;

std::map<char, std::function<void()>> comm_func = {
    {KEY_Q, command_quit},  {KEY_W, command_rotate}, {KEY_A, command_left},
    {KEY_D, command_right}, {KEY_S, command_down},
};

char getch() {
  char c;
  struct termios old, cur;
  tcgetattr(0, &cur);
  old = cur;
  cfmakeraw(&cur);
  tcsetattr(0, 0, &cur);
  c = getchar();
  tcsetattr(0, 0, &old);
  return c;
}

void key_event() {
  while (running) {
    command = getch();
    comm_func[command]();
  }
}

void start_listener() {
  std::thread t(key_event);
  t.detach();
}

void command_quit() { quit(); }
void command_rotate() { rotate(); }
void command_left() { left(); }
void command_right() { right(); }
void command_down() { down(); }
}  // namespace gm
