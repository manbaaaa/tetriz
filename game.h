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
#include "./tetromino.h"

namespace gm {
extern bool running;
extern int row, col;
extern Tetromino_1 cur;

extern Tetromino_2 cur_set;
extern int cur_index;

extern Tetromino_3 cur_s;

void quit();
void init();

void quit();
void rotate();
void left();
void right();
void down();

}  // namespace gm
