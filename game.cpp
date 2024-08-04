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
Tetromino cur;
void init() {
  running = true;
  row = 2;
  col = 15;
  cur = O;
}
void quit() { running = false; }
void rotate() { cur = rotate(cur); }
void left() { col--; }
void right() { col++; }
void down() { row++; }
}  // namespace gm
