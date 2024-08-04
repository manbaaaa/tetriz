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

#include "./utils.h"
#include "./define.h"

namespace ut {
int fps() {
  // static mean the variable will be initialized only once
  static auto start = std::chrono::system_clock::now();
  auto end = start;
  static int frame_count = 0;
  static int fps = 0;

  end = std::chrono::system_clock::now();
  frame_count++;
  if (end - start > std::chrono::seconds(1)) {
    fps = frame_count;
    frame_count = 0;
    start = end;
  }

  return fps;
}

std::string utf32_to_utf8(std::u32string str) {
  static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
  return convert.to_bytes(str);
}

}  // namespace ut
