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
Tetromino_1 I = {{0, 0, 0, 0, 0},
                 {0, 0, 0, 0, 0},
                 {0, 1, 1, 1, 1},
                 {0, 0, 0, 0, 0},
                 {0, 0, 0, 0, 0}};
Tetromino_1 J = {{2, 0, 0}, {2, 2, 2}, {0, 0, 0}};
Tetromino_1 L = {{0, 0, 3}, {3, 3, 3}, {0, 0, 0}};
Tetromino_1 O = {{0, 4, 4}, {0, 4, 4}, {0, 0, 0}};
Tetromino_1 S = {{0, 5, 5}, {5, 5, 0}, {0, 0, 0}};
Tetromino_1 T = {{0, 6, 0}, {6, 6, 6}, {0, 0, 0}};
Tetromino_1 Z = {{7, 7, 0}, {0, 7, 7}, {0, 0, 0}};

std::map<int, Color> tetro_color = {{1, Color::CYAN},   {2, Color::BLUE},
                                    {3, Color::ORANGE}, {4, Color::YELLOW},
                                    {5, Color::GREEN},  {6, Color::PURPLE},
                                    {7, Color::RED}};

Tetromino_1 rotate(const Tetromino_1& t) {
  Tetromino_1 result(t.size(), std::vector<int>(t.size(), 0));
  for (int i = 0; i < t.size(); i++) {
    for (int j = 0; j < t[0].size(); j++) {
      if (t[i][j] > 0) {
        result[j][t.size() - 1 - i] = t[i][j];
      }
    }
  }
  return std::move(result);
}

// -----------------------------------------------------------

/**
 * I
 * 0000   0010   0000   0100
 * 1111   0010   0000   0100
 * 0000   0010   1111   0100
 * 0000   0010   0000   0100
 *
 * 0x0f00 0x2222 0x00f0 0x4444
 *
 * [0000 0000] << name
 * [0000 0000] << Color
 * [0000 0000] << data
 * [0000 0000] << data
 *
 */
Tetromino_2 I_set = {0x0f00 | static_cast<int>(Color::CYAN) << 16 | 'I' << 24,
                     0x2222 | static_cast<int>(Color::CYAN) << 16 | 'I' << 24,
                     0x00f0 | static_cast<int>(Color::CYAN) << 16 | 'I' << 24,
                     0x4444 | static_cast<int>(Color::CYAN) << 16 | 'I' << 24};

/**
 *                 ^ y
 *                 |
 *                 |
 *                 |
 * -------------0--0--0--0---------------> x
 *
 */
Tetromino_3 i = {{
    {{{'I', static_cast<int>(Color::CYAN)}, {-1, 0}, {1, 0}, {2, 0}}},   // 0
    {{{'I', static_cast<int>(Color::CYAN)}, {0, 1}, {0, -1}, {0, -2}}},  // R
    {{{'I', static_cast<int>(Color::CYAN)}, {-2, 0}, {-1, 0}, {1, 0}}},  // 2
    {{{'I', static_cast<int>(Color::CYAN)}, {0, 2}, {0, 1}, {0, -1}}}    // L
}};

Tetromino_3 j = {{
    {{{'J', static_cast<int>(Color::BLUE)}, {-1, 1}, {-1, 0}, {1, 0}}},  // 0
    {{{'J', static_cast<int>(Color::BLUE)}, {1, 1}, {0, 1}, {0, -1}}},   // R
    {{{'J', static_cast<int>(Color::BLUE)}, {-1, 0}, {1, 0}, {1, -1}}},  // 2
    {{{'J', static_cast<int>(Color::BLUE)}, {0, 1}, {-1, -1}, {0, -1}}}  // L
}};

Tetromino_3 l = {{
    {{{'L', static_cast<int>(Color::ORANGE)}, {-1, 0}, {1, 0}, {1, 1}}},    // 0
    {{{'L', static_cast<int>(Color::ORANGE)}, {0, 1}, {0, -1}, {1, -1}}},   // R
    {{{'L', static_cast<int>(Color::ORANGE)}, {-1, -1}, {-1, 0}, {1, 0}}},  // 2
    {{{'L', static_cast<int>(Color::ORANGE)}, {-1, 1}, {0, 1}, {0, -1}}}    // L
}};

Tetromino_3 o = {{
    {{{'O', static_cast<int>(Color::YELLOW)}, {0, 1}, {1, 1}, {1, 0}}},    // 0
    {{{'O', static_cast<int>(Color::YELLOW)}, {0, -1}, {1, 0}, {1, -1}}},  // R
    {{{'O', static_cast<int>(Color::YELLOW)},
      {-1, -1},
      {-1, 0},
      {0, -1}}},                                                          // 2
    {{{'O', static_cast<int>(Color::YELLOW)}, {-1, 1}, {-1, 0}, {0, 1}}}  // L
}};
Tetromino_3 s = {{
    {{{'S', static_cast<int>(Color::GREEN)}, {-1, 0}, {0, 1}, {1, 1}}},    // 0
    {{{'S', static_cast<int>(Color::GREEN)}, {0, 1}, {1, 0}, {1, -1}}},    // R
    {{{'S', static_cast<int>(Color::GREEN)}, {-1, -1}, {0, -1}, {1, 0}}},  // 2
    {{{'S', static_cast<int>(Color::GREEN)}, {-1, 1}, {-1, 0}, {0, -1}}}   // L
}};
Tetromino_3 t = {{
    {{{'T', static_cast<int>(Color::PURPLE)}, {-1, 0}, {0, 1}, {1, 0}}},   // 0
    {{{'T', static_cast<int>(Color::PURPLE)}, {0, 1}, {1, 0}, {0, -1}}},   // R
    {{{'T', static_cast<int>(Color::PURPLE)}, {-1, 0}, {1, 0}, {0, -1}}},  // 2
    {{{'T', static_cast<int>(Color::PURPLE)}, {-1, 0}, {0, 1}, {0, -1}}}   // L
}};
Tetromino_3 z = {{
    {{{'Z', static_cast<int>(Color::RED)}, {-1, 1}, {0, 1}, {1, 0}}},    // 0
    {{{'Z', static_cast<int>(Color::RED)}, {1, 1}, {1, 0}, {0, -1}}},    // R
    {{{'Z', static_cast<int>(Color::RED)}, {-1, 0}, {0, -1}, {1, -1}}},  // 2
    {{{'Z', static_cast<int>(Color::RED)}, {-1, -1}, {-1, 0}, {0, 1}}}   // L
}};
}  // namespace gm
