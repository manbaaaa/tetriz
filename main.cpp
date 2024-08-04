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
#include "./draw.h"
#include "./game.h"
#include "./terminal.h"
#include "./utils.h"

void init() {
  tc::hide_cursor();
  gm::start_listener();
  gm::init();
}

void loop() {
  int i = 1;
  while (gm::running) {
    tc::clear_screen();
    tc::hide_cursor();
    dw::window(1, 1, 9, 6, "Hold");
    dw::window(1, 10, 12, 22, "Tetriz");
    dw::window(7, 1, 9, 16, "Status");
    dw::window(19, 22, 8, 4, "Info");
    dw::window(1, 22, 8, 18, "Next");

    int fps = ut::fps();
    tc::move_to(10, 4);
    std::cout << "FPS: " << fps << std::flush;
    tc::move_to(gm::row, ut::block2col(gm::col));
    // tc::set_back_color(15);
    // std::cout << "  ";

    dw::tetromino(gm::cur, gm::row, gm::col);
    tc::reset_color();

    std::cout << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void exit() {
  tc::show_cursor();
  tc::reset_color();
  tc::clear_screen();
  tc::move_to(1, 1);
  tc::set_fore_color(9);
  std::cout << "Bye!" << std::endl;
}

int main() {
  init();
  loop();
  exit();

  return 0;
}
