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

int main() {
  // std::cout << "\033[5;10H" << "\033[38;5;214m" << "helloworld" <<
  // "\033[10;1H"; tc::move_to(5, 10); tc::set_fore_color(214); std::cout <<
  // "helloworld"; tc::move_to(10, 1); tc::reset_color();
  tc::hide_cursor();
  int i = 1;
  while (true) {
    /* code */
    tc::clear_screen();
    tc::move_to(i++, 10);
    tc::set_back_color(15);
    std::cout << "  ";
    tc::reset_color();
    std::cout << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}
