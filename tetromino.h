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
#include "./color.h"
#include "./define.h"

namespace gm {
// I[5][5] JLOSTZ[3][3]
using Tetromino_1 = std::vector<std::vector<int>>;
extern Tetromino_1 I, J, L, O, S, T, Z;
extern Tetromino_1 rotate(const Tetromino_1& t);
extern std::map<int, Color> tetro_color;

// -----------------------------------------------------------
using Tetromino_2 = std::array<int, 4>;
extern Tetromino_2 I_set, J_set, L_set, O_set, S_set, T_set, Z_set;

// -----------------------------------------------------------
using Tetromino_3 = std::array<std::array<std::pair<int, int>, 4>, 4>;
extern Tetromino_3 i, j, l, o, s, t, z;
}  // namespace gm
