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

#ifndef TERMINAL_H_
#define TERMINAL_H_

namespace tc {  // terminal control
void move_to(int row, int col);
void set_fore_color(int id);
void set_back_color(int id);
void clear_screen();
void reset_color();
void hide_cursor();
void show_cursor();
}  // namespace tc

#endif  // TERMINAL_H_
