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

#include "./terminal.h"
#include <iostream>

#define CSI "\033["

void tc::move_to(int row, int col) {
  std::cout << CSI << row << ";" << col << "H";
}

void tc::set_fore_color(int id) { std::cout << CSI << "38;5;" << id << "m"; }

void tc::set_back_color(int id) { std::cout << CSI << "48;5;" << id << "m"; }

void tc::clear_screen() { std::cout << CSI << "2J"; }

void tc::reset_color() { std::cout << CSI << "0m"; }

void tc::hide_cursor() { std::cout << CSI << "?25l"; }

void tc::show_cursor() { std::cout << CSI << "?25h"; }
