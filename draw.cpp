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
#include <chrono>
#include <string>

#include "./control.h"
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
constexpr int CONTROLS_TOP = 16;
constexpr int CONTROLS_LEFT = 27;
constexpr int REQUIRED_ROWS = 23;
constexpr int REQUIRED_COLS = 76;
constexpr int FEEDBACK_FLASH_MS = 90;
DisplayOptions display_options;
std::array<std::array<std::string, gm::BOARD_WIDTH>, gm::BOARD_HEIGHT>
    last_board_cells = {};
std::deque<gm::PieceType> last_next_queue;
std::array<std::string, 12> last_status_lines = {};
std::optional<gm::PieceType> last_hold;
std::optional<gm::Phase> last_overlay_phase;
bool hold_initialized = false;
bool last_hold_available = true;

void reset_cached_render_state() {
  last_board_cells = {};
  last_next_queue.clear();
  last_status_lines = {};
  last_hold.reset();
  last_overlay_phase.reset();
  hold_initialized = false;
  last_hold_available = true;
}

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

void clear_area(int top, int left, int width, int height) {
  const std::string blank(width * 2, ' ');
  for (int row = 0; row < height; row++) {
    text(top + row, left, blank);
  }
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

void draw_labeled_piece_preview(gm::PieceType type, int top, int left,
                                int label_offset) {
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      empty_block(top + row + 1, left + col);
    }
  }

  gm::Piece piece{type, top + 2, left + 2, 0};
  for (const auto& point : gm::blocks_for(piece)) {
    block(point.row, point.col, gm::piece_color(type));
  }
  text_fixed(top + 2, left + label_offset, std::string(1, gm::piece_name(type)),
             1);
}

void draw_piece_preview(gm::PieceType type, int top, int left) {
  draw_labeled_piece_preview(type, top, left, 6);
}

void draw_board(const gm::Snapshot& snapshot) {
  std::array<std::array<bool, gm::BOARD_WIDTH>, gm::BOARD_HEIGHT> ghost = {};
  if (display_options.show_ghost) {
    for (const auto& point : gm::blocks_for(snapshot.ghost)) {
      if (point.row >= 0 && point.row < gm::BOARD_HEIGHT && point.col >= 0 &&
          point.col < gm::BOARD_WIDTH) {
        ghost[point.row][point.col] = true;
      }
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
      } else if (display_options.show_ghost && ghost[row][col]) {
        cell_key = "g";
      } else {
        cell_key = "e";
      }

      if (last_board_cells[row][col] == cell_key) {
        continue;
      }
      last_board_cells[row][col] = cell_key;

      if (active[row][col]) {
        block(screen_row, screen_col, gm::piece_color(snapshot.current.type));
      } else if (snapshot.board[row][col].filled) {
        block(screen_row, screen_col, snapshot.board[row][col].color);
      } else if (display_options.show_ghost && ghost[row][col]) {
        ghost_block(screen_row, screen_col);
      } else {
        empty_block(screen_row, screen_col);
      }
    }
  }
}

void draw_hold(const gm::Snapshot& snapshot) {
  if (hold_initialized && last_hold == snapshot.hold &&
      last_hold_available == snapshot.hold_available) {
    return;
  }
  hold_initialized = true;
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
  if (last_next_queue == queue) {
    return;
  }
  last_next_queue = queue;

  const int count = std::min<int>(3, queue.size());
  clear_area(NEXT_TOP + 1, NEXT_LEFT + 1, 9, 12);
  for (int i = 0; i < count; i++) {
    draw_labeled_piece_preview(queue[i], NEXT_TOP + 1 + i * 4,
                               NEXT_LEFT + 1, 7);
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

std::string t_spin_label(gm::TSpinType type) {
  switch (type) {
    case gm::TSpinType::None:
      return "no";
    case gm::TSpinType::Mini:
      return "mini";
    case gm::TSpinType::Full:
      return "full";
  }
  return "no";
}

void draw_status(const gm::Snapshot& snapshot, int fps) {
  const std::array<std::string, 12> lines = {
      "Score " + std::to_string(snapshot.score),
      "High  " + std::to_string(snapshot.high_score),
      "Lv " + std::to_string(snapshot.level) + " Lines " +
          std::to_string(snapshot.lines),
      "",
      "Combo " + (snapshot.combo >= 0 ? std::to_string(snapshot.combo) : "-"),
      std::string("B2B   ") + (snapshot.back_to_back ? "on" : "off"),
      std::string("PC    ") + (snapshot.perfect_clear ? "yes" : "no"),
      "TSpin " + t_spin_label(snapshot.t_spin),
      "",
      phase_label(snapshot.phase),
      display_options.show_fps ? "FPS   " + std::to_string(fps) : "",
      snapshot.hold_available ? "Hold ready" : "Hold locked",
  };
  for (int i = 0; i < static_cast<int>(lines.size()); i++) {
    if (last_status_lines[i] == lines[i]) {
      continue;
    }
    last_status_lines[i] = lines[i];
    text_fixed(STATUS_TOP + 1 + i, STATUS_LEFT + 1, lines[i], 18);
  }
}

void draw_info() {
  text_fixed(CONTROLS_TOP + 1, CONTROLS_LEFT + 1, "Move  A/D Arrows", 18);
  text_fixed(CONTROLS_TOP + 2, CONTROLS_LEFT + 1, "Drop  S Space", 18);
  text_fixed(CONTROLS_TOP + 3, CONTROLS_LEFT + 1, "Rot   W/Z/X Up", 18);
  text_fixed(CONTROLS_TOP + 4, CONTROLS_LEFT + 1, "Hold  C", 18);
  text_fixed(CONTROLS_TOP + 5, CONTROLS_LEFT + 1, "P/R/Q pause rst", 18);
}

void draw_overlay(gm::Phase phase) {
  const gm::Snapshot snapshot = gm::snapshot();
  if (last_overlay_phase.has_value() && *last_overlay_phase == phase &&
      phase != gm::Phase::GameOver) {
    return;
  }
  last_overlay_phase = phase;

  if (phase == gm::Phase::Playing) {
    return;
  }
  clear_area(BOARD_TOP + 7, BOARD_LEFT + 2, gm::BOARD_WIDTH, 12);
  const std::string line1 = phase == gm::Phase::Paused ? "PAUSED" : "GAME OVER";
  const std::string line2 = phase == gm::Phase::Paused ? "P to resume" : "R to restart";
  text_fixed(BOARD_TOP + 9, BOARD_LEFT + 3, line1, 12);
  text_fixed(BOARD_TOP + 11, BOARD_LEFT + 2, line2, 14);
  if (phase == gm::Phase::GameOver) {
    text_fixed(BOARD_TOP + 12, BOARD_LEFT + 2,
               "S " + std::to_string(snapshot.score), 14);
    text_fixed(BOARD_TOP + 13, BOARD_LEFT + 2,
               "L" + std::to_string(snapshot.level) + " R" +
                   std::to_string(snapshot.lines),
               14);
    for (int i = 0; i < 3; i++) {
      const auto& entry = snapshot.high_scores[i];
      text_fixed(BOARD_TOP + 15 + i, BOARD_LEFT + 2,
                 "#" + std::to_string(i + 1) + " " +
                     std::to_string(entry.score) + " L" +
                     std::to_string(entry.level) + " R" +
                     std::to_string(entry.lines),
                 14);
    }
  }
}

void board_border(std::string title) {
  const int screen_col = ut::block2col(BOARD_LEFT);
  const int total_width = (gm::BOARD_WIDTH + 2) * 2;
  const std::string horizontal(total_width - 2, '-');

  tc::move_to(BOARD_TOP, screen_col);
  std::cout << "+" << horizontal << "+";
  for (int row = 1; row < gm::BOARD_HEIGHT + 1; row++) {
    tc::move_to(BOARD_TOP + row, screen_col);
    std::cout << "|";
    tc::move_to(BOARD_TOP + row, screen_col + total_width - 1);
    std::cout << "|";
  }
  tc::move_to(BOARD_TOP + gm::BOARD_HEIGHT + 1, screen_col);
  std::cout << "+" << horizontal << "+";
  tc::move_to(BOARD_TOP, screen_col + (total_width - title.length()) / 2);
  std::cout << title;
}

void draw_feedback_border() {
  static bool flashing = false;
  static auto flash_until = std::chrono::steady_clock::time_point{};
  static bool last_visible = false;

  if (gm::consume_feedback_pulse()) {
    flashing = true;
    flash_until = std::chrono::steady_clock::now() +
                  std::chrono::milliseconds(FEEDBACK_FLASH_MS);
  }
  if (std::chrono::steady_clock::now() >= flash_until) {
    flashing = false;
  }
  if (last_visible == flashing) {
    return;
  }
  last_visible = flashing;

  if (flashing) {
    tc::set_fore_color(14);
  }
  board_border("Tetriz");
  tc::reset_color();
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

void set_display_options(DisplayOptions options) {
  display_options = options;
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
  static gm::Phase last_phase = gm::Phase::Playing;
  const bool phase_returned_to_playing =
      snapshot.phase == gm::Phase::Playing && last_phase != gm::Phase::Playing;
  if (first_render || was_too_small || phase_returned_to_playing) {
    tc::clear_screen();
    tc::hide_cursor();
    reset_cached_render_state();
    window(HOLD_TOP, HOLD_LEFT, 9, 7, "Hold");
    window(BOARD_TOP, BOARD_LEFT, gm::BOARD_WIDTH + 2, gm::BOARD_HEIGHT + 2, "Tetriz");
    window(STATUS_TOP, STATUS_LEFT, 12, 15, "Status");
    window(NEXT_TOP, NEXT_LEFT, 11, 15, "Next");
    window(CONTROLS_TOP, CONTROLS_LEFT, 11, 7, "Keys");
    draw_info();
    first_render = false;
    was_too_small = false;
  }
  last_phase = snapshot.phase;

  draw_hold(snapshot);
  draw_next(snapshot);
  draw_status(snapshot, fps);
  draw_board(snapshot);
  draw_overlay(snapshot.phase);
  draw_feedback_border();
  tc::reset_color();
  std::cout << std::flush;
}
}  // namespace dw
