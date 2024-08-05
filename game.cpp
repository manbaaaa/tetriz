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

#include "./game.h"
#include "./tetromino.h"

namespace gm {
bool running;
int row, col;
Tetromino_1 cur;
Tetromino_2 cur_set;
Tetromino_3 cur_s;
int cur_index;
void init() {
  running = true;
  row = 2;
  col = 15;
  cur = O;
  cur_set = I_set;
  cur_index = 0;
  cur_s = t;
}
void quit() { running = false; }
void rotate() {
  // Tetromino_1
  // cur = rotate(cur);
  // Tetromino_2
  cur_index = (cur_index + 1) % 4;
}
void left() { col--; }
void right() { col++; }
void down() { row++; }
}  // namespace gm
