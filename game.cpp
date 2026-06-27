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

#include <algorithm>
#include <mutex>
#include <random>

namespace gm {
namespace {
std::mutex state_mutex;
Board board_state;
Piece current;
std::deque<PieceType> queue_state;
std::optional<PieceType> hold_state;
Phase phase_state = Phase::Playing;
int score_state = 0;
int level_state = 1;
int lines_state = 0;
bool hold_used = false;
std::chrono::steady_clock::time_point last_fall;

std::mt19937& rng() {
  static std::random_device seed;
  static std::mt19937 engine(seed());
  return engine;
}

Blocks base_blocks(PieceType type, int rotation) {
  const int r = rotation % 4;
  switch (type) {
    case PieceType::I:
      return r % 2 == 0 ? Blocks{{{0, -1}, {0, 0}, {0, 1}, {0, 2}}}
                        : Blocks{{{-1, 1}, {0, 1}, {1, 1}, {2, 1}}};
    case PieceType::J:
      switch (r) {
        case 0:
          return {{{-1, -1}, {0, -1}, {0, 0}, {0, 1}}};
        case 1:
          return {{{-1, 0}, {-1, 1}, {0, 0}, {1, 0}}};
        case 2:
          return {{{0, -1}, {0, 0}, {0, 1}, {1, 1}}};
        default:
          return {{{-1, 0}, {0, 0}, {1, -1}, {1, 0}}};
      }
    case PieceType::L:
      switch (r) {
        case 0:
          return {{{-1, 1}, {0, -1}, {0, 0}, {0, 1}}};
        case 1:
          return {{{-1, 0}, {0, 0}, {1, 0}, {1, 1}}};
        case 2:
          return {{{0, -1}, {0, 0}, {0, 1}, {1, -1}}};
        default:
          return {{{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}};
      }
    case PieceType::O:
      return {{{0, 0}, {0, 1}, {1, 0}, {1, 1}}};
    case PieceType::S:
      return r % 2 == 0 ? Blocks{{{-1, 0}, {-1, 1}, {0, -1}, {0, 0}}}
                        : Blocks{{{-1, 0}, {0, 0}, {0, 1}, {1, 1}}};
    case PieceType::T:
      switch (r) {
        case 0:
          return {{{-1, 0}, {0, -1}, {0, 0}, {0, 1}}};
        case 1:
          return {{{-1, 0}, {0, 0}, {0, 1}, {1, 0}}};
        case 2:
          return {{{0, -1}, {0, 0}, {0, 1}, {1, 0}}};
        default:
          return {{{-1, 0}, {0, -1}, {0, 0}, {1, 0}}};
      }
    case PieceType::Z:
      return r % 2 == 0 ? Blocks{{{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}}
                        : Blocks{{{-1, 1}, {0, 0}, {0, 1}, {1, 0}}};
  }
  return {};
}

void fill_bag() {
  std::array<PieceType, 7> bag = {PieceType::I, PieceType::J, PieceType::L,
                                  PieceType::O, PieceType::S, PieceType::T,
                                  PieceType::Z};
  std::shuffle(bag.begin(), bag.end(), rng());
  for (auto type : bag) {
    queue_state.push_back(type);
  }
}

void ensure_queue() {
  while (queue_state.size() < PREVIEW_COUNT + 1) {
    fill_bag();
  }
}

Piece make_piece(PieceType type) { return Piece{type, 1, BOARD_WIDTH / 2 - 1, 0}; }

bool collides(const Piece& piece) {
  for (const auto& block : blocks_for(piece)) {
    if (block.col < 0 || block.col >= BOARD_WIDTH || block.row >= BOARD_HEIGHT) {
      return true;
    }
    if (block.row >= 0 && board_state[block.row][block.col].filled) {
      return true;
    }
  }
  return false;
}

Piece ghost_piece_unlocked() {
  Piece ghost = current;
  while (!collides(Piece{ghost.type, ghost.row + 1, ghost.col, ghost.rotation})) {
    ghost.row++;
  }
  return ghost;
}

bool try_move(int row_delta, int col_delta) {
  Piece moved = current;
  moved.row += row_delta;
  moved.col += col_delta;
  if (collides(moved)) {
    return false;
  }
  current = moved;
  return true;
}

void try_rotate_steps(int steps) {
  if (phase_state != Phase::Playing || current.type == PieceType::O) {
    return;
  }
  Piece rotated = current;
  rotated.rotation = (rotated.rotation + steps + 4) % 4;
  const std::array<int, 5> kicks = {0, -1, 1, -2, 2};
  for (int kick : kicks) {
    Piece candidate = rotated;
    candidate.col += kick;
    if (!collides(candidate)) {
      current = candidate;
      return;
    }
  }
}

int clear_lines() {
  int cleared = 0;
  for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
    bool full = true;
    for (const auto& cell : board_state[row]) {
      if (!cell.filled) {
        full = false;
        break;
      }
    }
    if (!full) {
      continue;
    }
    cleared++;
    for (int move_row = row; move_row > 0; move_row--) {
      board_state[move_row] = board_state[move_row - 1];
    }
    board_state[0] = {};
    row++;
  }
  return cleared;
}

void apply_score(int cleared, int drop_bonus) {
  static const std::array<int, 5> line_scores = {0, 100, 300, 500, 800};
  score_state += drop_bonus;
  if (cleared > 0) {
    score_state += line_scores[cleared] * level_state;
    lines_state += cleared;
    level_state = lines_state / 10 + 1;
  }
}

void spawn_next() {
  ensure_queue();
  current = make_piece(queue_state.front());
  queue_state.pop_front();
  ensure_queue();
  hold_used = false;
  if (collides(current)) {
    phase_state = Phase::GameOver;
  }
}

void lock_piece(int drop_bonus = 0) {
  const Color color = piece_color(current.type);
  for (const auto& block : blocks_for(current)) {
    if (block.row >= 0 && block.row < BOARD_HEIGHT && block.col >= 0 &&
        block.col < BOARD_WIDTH) {
      board_state[block.row][block.col] = Cell{true, color};
    }
  }
  const int cleared = clear_lines();
  apply_score(cleared, drop_bonus);
  spawn_next();
}

int fall_interval_value() { return std::max(100, 800 - (level_state - 1) * 60); }

void reset_game() {
  board_state = {};
  queue_state.clear();
  hold_state.reset();
  phase_state = Phase::Playing;
  score_state = 0;
  level_state = 1;
  lines_state = 0;
  hold_used = false;
  running = true;
  ensure_queue();
  spawn_next();
  last_fall = std::chrono::steady_clock::now();
}
}  // namespace

std::atomic_bool running = false;

void init() {
  std::lock_guard<std::mutex> lock(state_mutex);
  reset_game();
}

void quit() {
  running = false;
}

void tick(std::chrono::steady_clock::time_point now) {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (phase_state != Phase::Playing) {
    last_fall = now;
    return;
  }
  if (now - last_fall < std::chrono::milliseconds(fall_interval_value())) {
    return;
  }
  if (!try_move(1, 0)) {
    lock_piece();
  }
  last_fall = now;
}

void rotate() {
  std::lock_guard<std::mutex> lock(state_mutex);
  try_rotate_steps(1);
}

void rotate_counterclockwise() {
  std::lock_guard<std::mutex> lock(state_mutex);
  try_rotate_steps(-1);
}

void rotate_180() {
  std::lock_guard<std::mutex> lock(state_mutex);
  try_rotate_steps(2);
}

void left() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (phase_state == Phase::Playing) {
    try_move(0, -1);
  }
}

void right() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (phase_state == Phase::Playing) {
    try_move(0, 1);
  }
}

void down() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (phase_state != Phase::Playing) {
    return;
  }
  if (try_move(1, 0)) {
    score_state += 1;
  } else {
    lock_piece();
  }
  last_fall = std::chrono::steady_clock::now();
}

void hard_drop() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (phase_state != Phase::Playing) {
    return;
  }
  int distance = 0;
  while (try_move(1, 0)) {
    distance++;
  }
  lock_piece(distance * 2);
  last_fall = std::chrono::steady_clock::now();
}

void hold() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (phase_state != Phase::Playing || hold_used) {
    return;
  }
  const PieceType previous = current.type;
  if (hold_state.has_value()) {
    current = make_piece(*hold_state);
    hold_state = previous;
    if (collides(current)) {
      phase_state = Phase::GameOver;
    }
  } else {
    hold_state = previous;
    spawn_next();
  }
  hold_used = true;
}

void toggle_pause() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (phase_state == Phase::Playing) {
    phase_state = Phase::Paused;
  } else if (phase_state == Phase::Paused) {
    phase_state = Phase::Playing;
    last_fall = std::chrono::steady_clock::now();
  }
}

void restart() {
  std::lock_guard<std::mutex> lock(state_mutex);
  reset_game();
}

Snapshot snapshot() {
  std::lock_guard<std::mutex> lock(state_mutex);
  return Snapshot{board_state, current,      ghost_piece_unlocked(), queue_state,
                  hold_state,  phase_state, score_state,            level_state,
                  lines_state, fall_interval_value(), !hold_used};
}

Blocks blocks_for(const Piece& piece) {
  Blocks blocks = base_blocks(piece.type, piece.rotation);
  for (auto& block : blocks) {
    block.row += piece.row;
    block.col += piece.col;
  }
  return blocks;
}

Color piece_color(PieceType type) {
  switch (type) {
    case PieceType::I:
      return Color::CYAN;
    case PieceType::J:
      return Color::BLUE;
    case PieceType::L:
      return Color::ORANGE;
    case PieceType::O:
      return Color::YELLOW;
    case PieceType::S:
      return Color::GREEN;
    case PieceType::T:
      return Color::PURPLE;
    case PieceType::Z:
      return Color::RED;
  }
  return Color::BLACK;
}

char piece_name(PieceType type) {
  switch (type) {
    case PieceType::I:
      return 'I';
    case PieceType::J:
      return 'J';
    case PieceType::L:
      return 'L';
    case PieceType::O:
      return 'O';
    case PieceType::S:
      return 'S';
    case PieceType::T:
      return 'T';
    case PieceType::Z:
      return 'Z';
  }
  return '?';
}

}  // namespace gm
