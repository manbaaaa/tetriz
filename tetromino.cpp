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

#include "./tetromino.h"

namespace gm {
// I[5][5] JLOSTZ[3][3]
Tetromino I = {{0, 0, 0, 0, 0},
               {0, 0, 0, 0, 0},
               {0, 1, 1, 1, 1},
               {0, 0, 0, 0, 0},
               {0, 0, 0, 0, 0}};
Tetromino J = {{2, 0, 0}, {2, 2, 2}, {0, 0, 0}};
Tetromino L = {{0, 0, 3}, {3, 3, 3}, {0, 0, 0}};
Tetromino O = {{0, 4, 4}, {0, 4, 4}, {0, 0, 0}};
Tetromino S = {{0, 5, 5}, {5, 5, 0}, {0, 0, 0}};
Tetromino T = {{0, 6, 0}, {6, 6, 6}, {0, 0, 0}};
Tetromino Z = {{7, 7, 0}, {0, 7, 7}, {0, 0, 0}};

std::map<int, Color> tetro_color = {{1, Color::CYAN},   {2, Color::BLUE},
                                    {3, Color::ORANGE}, {4, Color::YELLOW},
                                    {5, Color::GREEN},  {6, Color::PURPLE},
                                    {7, Color::RED}};

Tetromino rotate(const Tetromino& t) {
  Tetromino result(t.size(), std::vector<int>(t.size(), 0));
  for (int i = 0; i < t.size(); i++) {
    for (int j = 0; j < t[0].size(); j++) {
      if (t[i][j] > 0) {
        result[j][t.size() - 1 - i] = t[i][j];
      }
    }
  }
  return std::move(result);
}
}  // namespace gm
