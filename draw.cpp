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
#include "./draw.h"
#include <iostream>
#include <string>
#include "./terminal.h"
#include "./utils.h"
/**
*  	    0	1	2	3	4	5	6	7	8
9	A	B	C	D	E	F U+250x	─	━
│	┃	┄	┅	┆	┇	┈	┉	┊	┋
┌	┍	┎	┏ U+251x	┐	┑	┒	┓	└
┕	┖	┗	┘	┙	┚	┛	├	┝	┞
┟ U+252x	┠	┡	┢	┣	┤	┥	┦	┧
┨	┩	┪	┫	┬	┭	┮	┯ U+253x	┰
┱	┲	┳	┴	┵	┶	┷	┸	┹	┺
┻	┼	┽	┾	┿ U+254x	╀	╁	╂	╃
╄	╅	╆	╇	╈	╉	╊	╋	╌	╍
╎	╏ U+255x	═	║	╒	╓	╔	╕	╖
╗	╘	╙	╚	╛	╜	╝	╞	╟ U+256x
╠	╡	╢	╣	╤	╥	╦	╧	╨	╩
╪	╫	╬	╭	╮	╯ U+257x	╰	╱	╲
╳	╴	╵	╶	╷	╸	╹	╺	╻	╼
╽	╾	╿
*/

namespace dw {
const std::u32string style1 = U" ╔╗╚╝║═";
const std::u32string style2 = U" ┌┐└┘│─";
const std::u32string style3 = U" ┏┓┗┛┃━";
const std::u32string style4 = U" ╭╮╰╯│─";
const std::u32string cur_style = style3;

inline int block2col(int b) { return 2 * b - 1; }

void window(int top, int left, int width, int height, std::string title) {
  for (int r = 0; r < height; r++) {
    tc::move_to(top + r, block2col(left));
    for (int c = 0; c < width; c++) {
      if (r == 0 && c == 0) {
        std::cout << ut::utf32_to_utf8({cur_style[0], cur_style[1]});
      } else if (r == 0 && c == width - 1) {
        std::cout << ut::utf32_to_utf8({cur_style[2], cur_style[0]});
      } else if (r == height - 1 && c == 0) {
        std::cout << ut::utf32_to_utf8({cur_style[0], cur_style[3]});
      } else if (r == height - 1 && c == width - 1) {
        std::cout << ut::utf32_to_utf8({cur_style[4], cur_style[0]});
      } else if (r == 0 || r == height - 1) {
        std::cout << ut::utf32_to_utf8({cur_style[6], cur_style[6]});
      } else if (c == 0) {
        std::cout << ut::utf32_to_utf8({cur_style[0], cur_style[5]});
      } else if (c == width - 1) {
        std::cout << ut::utf32_to_utf8({cur_style[5], cur_style[0]});
      } else {
        std::cout << ut::utf32_to_utf8({cur_style[0], cur_style[0]});
      }
    }
  }
  // title
  tc::move_to(top, block2col(left) + (width * 2 - title.length()) / 2);
  std::cout << title;
}
}  // namespace dw
