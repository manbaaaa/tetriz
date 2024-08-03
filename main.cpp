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

#include <chrono>
#include <iostream>
#include <thread>
#include "./terminal.h"
#include "./utils.h"

void init() { tc::hide_cursor(); }

void loop() {
  int i = 1;
  while (true) {
    int fps = ut::fps();
    tc::clear_screen();
    tc::move_to(1, 1);
    std::cout << "FPS: " << fps << std::flush;
    tc::move_to(i++ % 20, 10);
    tc::set_back_color(15);
    std::cout << "  ";
    tc::reset_color();
    std::cout << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void exit() {
  tc::show_cursor();
  tc::reset_color();
}

int main() {
  init();
  loop();
  exit();

  return 0;
}
