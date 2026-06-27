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

#include <algorithm>
#include <string>

#include "./terminal.h"
#include "./utils.h"

namespace dw {
namespace {
constexpr int HOLD_TOP = 1;
constexpr int HOLD_LEFT = 1;
constexpr int BOARD_TOP = 1;
constexpr int BOARD_LEFT = 14;
constexpr int STATUS_TOP = 8;
constexpr int STATUS_LEFT = 1;
constexpr int NEXT_TOP = 1;
constexpr int NEXT_LEFT = 27;
constexpr int REQUIRED_ROWS = 22;
constexpr int REQUIRED_COLS = 70;

void block(int row, int col, Color color) {
  tc::move_to(row, ut::block2col(col));
  tc::set_back_color(static_cast<int>(color));
  std::cout << "  ";
  tc::reset_color();
}

void ghost_block(int row, int col) {
  tc::move_to(row, ut::block2col(col));
  tc::set_fore_color(8);
  std::cout << "[]";
  tc::reset_color();
}

void empty_block(int row, int col) {
  tc::move_to(row, ut::block2col(col));
  tc::reset_color();
  std::cout << "  ";
}

void text(int row, int col, const std::string& value) {
  tc::move_to(row, ut::block2col(col));
  std::cout << value;
}

void text_fixed(int row, int col, const std::string& value, int width) {
  std::string padded = value.substr(0, width);
  padded.append(width - padded.length(), ' ');
  text(row, col, padded);
}

bool terminal_too_small(tc::Size size) {
  return size.rows > 0 && size.cols > 0 &&
         (size.rows < REQUIRED_ROWS || size.cols < REQUIRED_COLS);
}

void draw_small_terminal_message(tc::Size size) {
  tc::clear_screen();
  tc::move_to(1, 1);
  std::cout << "Tetriz needs at least " << REQUIRED_COLS << "x" << REQUIRED_ROWS
            << ".";
  tc::move_to(2, 1);
  std::cout << "Current terminal: " << size.cols << "x" << size.rows << ".";
  tc::move_to(3, 1);
  std::cout << "Resize the terminal or press Q to quit.";
}

void draw_piece_preview(gm::PieceType type, int top, int left) {
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      empty_block(top + row + 1, left + col);
    }
  }

  gm::Piece piece{type, top + 2, left + 2, 0};
  for (const auto& point : gm::blocks_for(piece)) {
    block(point.row, point.col, gm::piece_color(type));
  }
  text_fixed(top + 2, left + 6, std::string(1, gm::piece_name(type)), 1);
}

void draw_board(const gm::Snapshot& snapshot) {
  static std::array<std::array<std::string, gm::BOARD_WIDTH>, gm::BOARD_HEIGHT>
      last_cells = {};

  std::array<std::array<bool, gm::BOARD_WIDTH>, gm::BOARD_HEIGHT> ghost = {};
  for (const auto& point : gm::blocks_for(snapshot.ghost)) {
    if (point.row >= 0 && point.row < gm::BOARD_HEIGHT && point.col >= 0 &&
        point.col < gm::BOARD_WIDTH) {
      ghost[point.row][point.col] = true;
    }
  }

  std::array<std::array<bool, gm::BOARD_WIDTH>, gm::BOARD_HEIGHT> active = {};
  for (const auto& point : gm::blocks_for(snapshot.current)) {
    if (point.row >= 0 && point.row < gm::BOARD_HEIGHT && point.col >= 0 &&
        point.col < gm::BOARD_WIDTH) {
      active[point.row][point.col] = true;
    }
  }

  for (int row = 0; row < gm::BOARD_HEIGHT; row++) {
    for (int col = 0; col < gm::BOARD_WIDTH; col++) {
      const int screen_row = BOARD_TOP + 1 + row;
      const int screen_col = BOARD_LEFT + 1 + col;
      std::string cell_key;
      if (active[row][col]) {
        cell_key = "a" + std::to_string(static_cast<int>(gm::piece_color(snapshot.current.type)));
      } else if (snapshot.board[row][col].filled) {
        cell_key = "f" + std::to_string(static_cast<int>(snapshot.board[row][col].color));
      } else if (ghost[row][col]) {
        cell_key = "g";
      } else {
        cell_key = "e";
      }

      if (last_cells[row][col] == cell_key) {
        continue;
      }
      last_cells[row][col] = cell_key;

      if (active[row][col]) {
        block(screen_row, screen_col, gm::piece_color(snapshot.current.type));
      } else if (snapshot.board[row][col].filled) {
        block(screen_row, screen_col, snapshot.board[row][col].color);
      } else if (ghost[row][col]) {
        ghost_block(screen_row, screen_col);
      } else {
        empty_block(screen_row, screen_col);
      }
    }
  }
}

void draw_hold(const gm::Snapshot& snapshot) {
  static bool initialized = false;
  static std::optional<gm::PieceType> last_hold;
  static bool last_hold_available = true;
  if (initialized && last_hold == snapshot.hold &&
      last_hold_available == snapshot.hold_available) {
    return;
  }
  initialized = true;
  last_hold = snapshot.hold;
  last_hold_available = snapshot.hold_available;

  const auto& held = snapshot.hold;
  if (held.has_value()) {
    draw_piece_preview(*held, HOLD_TOP, HOLD_LEFT + 1);
    if (!snapshot.hold_available) {
      text_fixed(HOLD_TOP + 5, HOLD_LEFT + 2, "locked", 7);
    } else {
      text_fixed(HOLD_TOP + 5, HOLD_LEFT + 2, "C hold", 7);
    }
  } else {
    for (int row = 0; row < 4; row++) {
      for (int col = 0; col < 4; col++) {
        empty_block(HOLD_TOP + row + 2, HOLD_LEFT + col + 2);
      }
    }
    text_fixed(HOLD_TOP + 3, HOLD_LEFT + 2, "empty", 7);
    text_fixed(HOLD_TOP + 5, HOLD_LEFT + 2, "C hold", 7);
  }
}

void draw_next(const gm::Snapshot& snapshot) {
  const auto& queue = snapshot.next;
  static std::deque<gm::PieceType> last_queue;
  if (last_queue == queue) {
    return;
  }
  last_queue = queue;

  const int count = std::min<int>(gm::PREVIEW_COUNT, queue.size());
  for (int i = 0; i < count; i++) {
    draw_piece_preview(queue[i], NEXT_TOP + i * 5, NEXT_LEFT);
  }
}

std::string phase_label(gm::Phase phase) {
  switch (phase) {
    case gm::Phase::Playing:
      return "Playing";
    case gm::Phase::Paused:
      return "Paused";
    case gm::Phase::GameOver:
      return "Game Over";
  }
  return "";
}

void draw_status(const gm::Snapshot& snapshot, int fps) {
  const std::array<std::string, 7> lines = {
      "Score " + std::to_string(snapshot.score),
      "High  " + std::to_string(snapshot.high_score),
      "Lv " + std::to_string(snapshot.level) + " Lines " +
          std::to_string(snapshot.lines),
      "Combo " + (snapshot.combo >= 0 ? std::to_string(snapshot.combo) : "-"),
      std::string("B2B   ") + (snapshot.back_to_back ? "on" : "off"),
      "FPS   " + std::to_string(fps),
      phase_label(snapshot.phase),
  };
  static std::array<std::string, 7> last_lines = {};
  for (int i = 0; i < static_cast<int>(lines.size()); i++) {
    if (last_lines[i] == lines[i]) {
      continue;
    }
    last_lines[i] = lines[i];
    text_fixed(STATUS_TOP + 1 + i * 2, STATUS_LEFT + 1, lines[i], 18);
  }
}

void draw_info() {
  text_fixed(NEXT_TOP + 15, NEXT_LEFT + 1, "A/D or <-/->", 14);
  text_fixed(NEXT_TOP + 16, NEXT_LEFT + 1, "W or Up rot", 14);
  text_fixed(NEXT_TOP + 17, NEXT_LEFT + 1, "Z ccw X 180", 14);
  text_fixed(NEXT_TOP + 18, NEXT_LEFT + 1, "S or Down", 14);
  text_fixed(NEXT_TOP + 19, NEXT_LEFT + 1, "Space drop", 14);
  text_fixed(NEXT_TOP + 20, NEXT_LEFT + 1, "C hold", 14);
  text_fixed(NEXT_TOP + 21, NEXT_LEFT + 1, "P pause R rst", 14);
}

void draw_overlay(gm::Phase phase) {
  static std::optional<gm::Phase> last_phase;
  if (last_phase.has_value() && *last_phase == phase) {
    return;
  }
  last_phase = phase;

  if (phase == gm::Phase::Playing) {
    text_fixed(BOARD_TOP + 9, BOARD_LEFT + 3, "", 12);
    text_fixed(BOARD_TOP + 11, BOARD_LEFT + 2, "", 14);
    return;
  }
  const std::string line1 = phase == gm::Phase::Paused ? "PAUSED" : "GAME OVER";
  const std::string line2 = phase == gm::Phase::Paused ? "P to resume" : "R to restart";
  text_fixed(BOARD_TOP + 9, BOARD_LEFT + 3, line1, 12);
  text_fixed(BOARD_TOP + 11, BOARD_LEFT + 2, line2, 14);
}
}  // namespace

void window(int top, int left, int width, int height, std::string title) {
  const int screen_col = ut::block2col(left);
  const int total_width = width * 2;
  const std::string horizontal(total_width - 2, '-');
  const std::string blank(total_width - 2, ' ');

  tc::move_to(top, screen_col);
  std::cout << "+" << horizontal << "+";

  for (int r = 0; r < height; r++) {
    if (r == 0 || r == height - 1) {
      continue;
    }
    tc::move_to(top + r, screen_col);
    std::cout << "|" << blank << "|";
  }

  tc::move_to(top + height - 1, screen_col);
  std::cout << "+" << horizontal << "+";

  tc::move_to(top, screen_col + (total_width - title.length()) / 2);
  std::cout << title;
}

void render(int fps) {
  static bool was_too_small = false;
  const tc::Size terminal_size = tc::terminal_size();
  if (terminal_too_small(terminal_size)) {
    draw_small_terminal_message(terminal_size);
    was_too_small = true;
    std::cout << std::flush;
    return;
  }

  const gm::Snapshot snapshot = gm::snapshot();
  static bool first_render = true;
  if (first_render || was_too_small) {
    tc::clear_screen();
    tc::hide_cursor();
    window(HOLD_TOP, HOLD_LEFT, 9, 7, "Hold");
    window(BOARD_TOP, BOARD_LEFT, gm::BOARD_WIDTH + 2, gm::BOARD_HEIGHT + 2, "Tetriz");
    window(STATUS_TOP, STATUS_LEFT, 12, 15, "Status");
    window(NEXT_TOP, NEXT_LEFT, 9, 22, "Next");
    draw_info();
    first_render = false;
    was_too_small = false;
  }

  draw_hold(snapshot);
  draw_next(snapshot);
  draw_status(snapshot, fps);
  draw_board(snapshot);
  draw_overlay(snapshot.phase);
  tc::reset_color();
  std::cout << std::flush;
}
}  // namespace dw
