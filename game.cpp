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
#include <fstream>
#include <mutex>
#include <random>
#include <string>

namespace gm {
namespace {
std::mutex state_mutex;
constexpr int LOCK_DELAY_MS = 500;
constexpr int MAX_LOCK_RESETS = 15;

struct GameState {
  Board board;
  Piece current;
  std::deque<PieceType> queue;
  std::optional<PieceType> hold;
  Phase phase = Phase::Playing;
  int score = 0;
  int high_score = 0;
  int level = 1;
  int lines = 0;
  int combo = -1;
  bool back_to_back = false;
  bool perfect_clear = false;
  bool last_action_rotate = false;
  bool t_spin = false;
  bool hold_used = false;
  std::chrono::steady_clock::time_point last_fall;
  std::optional<std::chrono::steady_clock::time_point> lock_started_at;
  int lock_resets = 0;
  std::string high_score_path = ".tetriz_high_score";
};

GameState state;

std::mt19937& rng() {
  static std::random_device seed;
  static std::mt19937 engine(seed());
  return engine;
}

std::array<Point, 5> wall_kicks(PieceType type, int from, int to);

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
    state.queue.push_back(type);
  }
}

void ensure_queue() {
  while (state.queue.size() < PREVIEW_COUNT + 1) {
    fill_bag();
  }
}

Piece make_piece(PieceType type) { return Piece{type, 1, BOARD_WIDTH / 2 - 1, 0}; }

bool collides(const Piece& piece) {
  for (const auto& block : blocks_for(piece)) {
    if (block.col < 0 || block.col >= BOARD_WIDTH || block.row >= BOARD_HEIGHT) {
      return true;
    }
    if (block.row >= 0 && state.board[block.row][block.col].filled) {
      return true;
    }
  }
  return false;
}

Piece ghost_piece_unlocked() {
  Piece ghost = state.current;
  while (!collides(Piece{ghost.type, ghost.row + 1, ghost.col, ghost.rotation})) {
    ghost.row++;
  }
  return ghost;
}

bool try_move(int row_delta, int col_delta) {
  Piece moved = state.current;
  moved.row += row_delta;
  moved.col += col_delta;
  if (collides(moved)) {
    return false;
  }
  state.current = moved;
  return true;
}

bool grounded() {
  Piece dropped = state.current;
  dropped.row += 1;
  return collides(dropped);
}

bool occupied_or_wall(int row, int col) {
  if (col < 0 || col >= BOARD_WIDTH || row >= BOARD_HEIGHT) {
    return true;
  }
  return row >= 0 && state.board[row][col].filled;
}

bool is_t_spin() {
  if (!state.last_action_rotate || state.current.type != PieceType::T) {
    return false;
  }

  const int center_row = state.current.row;
  const int center_col = state.current.col;
  int occupied_corners = 0;
  for (const auto& corner : std::array<Point, 4>{
           Point{-1, -1}, Point{-1, 1}, Point{1, -1}, Point{1, 1}}) {
    if (occupied_or_wall(center_row + corner.row, center_col + corner.col)) {
      occupied_corners++;
    }
  }
  return occupied_corners >= 3;
}

void clear_lock_delay() {
  state.lock_started_at.reset();
  state.lock_resets = 0;
}

void maybe_reset_lock_delay() {
  if (grounded() && state.lock_started_at.has_value() &&
      state.lock_resets < MAX_LOCK_RESETS) {
    state.lock_started_at = std::chrono::steady_clock::now();
    state.lock_resets++;
  } else if (!grounded()) {
    clear_lock_delay();
  }
}

void try_rotate_steps(int steps) {
  if (state.phase != Phase::Playing || state.current.type == PieceType::O) {
    return;
  }
  Piece rotated = state.current;
  const int from = rotated.rotation;
  const int to = (rotated.rotation + steps + 4) % 4;
  rotated.rotation = to;
  for (const auto& kick : wall_kicks(state.current.type, from, to)) {
    Piece candidate = rotated;
    candidate.col += kick.col;
    candidate.row += kick.row;
    if (!collides(candidate)) {
      state.current = candidate;
      state.last_action_rotate = true;
      maybe_reset_lock_delay();
      return;
    }
  }
}

std::array<Point, 5> wall_kicks(PieceType type, int from, int to) {
  if (type == PieceType::I) {
    if (from == 0 && to == 1) return {{{0, 0}, {0, -2}, {0, 1}, {-1, -2}, {2, 1}}};
    if (from == 1 && to == 0) return {{{0, 0}, {0, 2}, {0, -1}, {1, 2}, {-2, -1}}};
    if (from == 1 && to == 2) return {{{0, 0}, {0, -1}, {0, 2}, {2, -1}, {-1, 2}}};
    if (from == 2 && to == 1) return {{{0, 0}, {0, 1}, {0, -2}, {-2, 1}, {1, -2}}};
    if (from == 2 && to == 3) return {{{0, 0}, {0, 2}, {0, -1}, {1, 2}, {-2, -1}}};
    if (from == 3 && to == 2) return {{{0, 0}, {0, -2}, {0, 1}, {-1, -2}, {2, 1}}};
    if (from == 3 && to == 0) return {{{0, 0}, {0, 1}, {0, -2}, {-2, 1}, {1, -2}}};
    if (from == 0 && to == 3) return {{{0, 0}, {0, -1}, {0, 2}, {2, -1}, {-1, 2}}};
  }

  if (from == 0 && to == 1) return {{{0, 0}, {0, -1}, {1, -1}, {-2, 0}, {-2, -1}}};
  if (from == 1 && to == 0) return {{{0, 0}, {0, 1}, {-1, 1}, {2, 0}, {2, 1}}};
  if (from == 1 && to == 2) return {{{0, 0}, {0, 1}, {-1, 1}, {2, 0}, {2, 1}}};
  if (from == 2 && to == 1) return {{{0, 0}, {0, -1}, {1, -1}, {-2, 0}, {-2, -1}}};
  if (from == 2 && to == 3) return {{{0, 0}, {0, 1}, {1, 1}, {-2, 0}, {-2, 1}}};
  if (from == 3 && to == 2) return {{{0, 0}, {0, -1}, {-1, -1}, {2, 0}, {2, -1}}};
  if (from == 3 && to == 0) return {{{0, 0}, {0, -1}, {-1, -1}, {2, 0}, {2, -1}}};
  if (from == 0 && to == 3) return {{{0, 0}, {0, 1}, {1, 1}, {-2, 0}, {-2, 1}}};
  return {{{0, 0}, {0, -1}, {0, 1}, {0, -2}, {0, 2}}};
}

int clear_lines() {
  int cleared = 0;
  for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
    bool full = true;
    for (const auto& cell : state.board[row]) {
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
      state.board[move_row] = state.board[move_row - 1];
    }
    state.board[0] = {};
    row++;
  }
  return cleared;
}

bool board_empty() {
  for (const auto& row : state.board) {
    for (const auto& cell : row) {
      if (cell.filled) {
        return false;
      }
    }
  }
  return true;
}

void apply_score(int cleared, int drop_bonus, bool t_spin) {
  static const std::array<int, 5> line_scores = {0, 100, 300, 500, 800};
  static const std::array<int, 4> t_spin_scores = {400, 800, 1200, 1600};
  state.score += drop_bonus;
  state.t_spin = t_spin;
  if (cleared > 0) {
    const bool difficult_clear = cleared == 4 || t_spin;
    const int base_score = t_spin ? t_spin_scores[cleared] : line_scores[cleared];
    int line_score = base_score * state.level;
    if (difficult_clear && state.back_to_back) {
      line_score += line_score / 2;
    }
    state.combo++;
    if (state.combo > 0) {
      line_score += state.combo * 50 * state.level;
    }
    state.score += line_score;
    state.lines += cleared;
    state.level = state.lines / 10 + 1;
    state.back_to_back = difficult_clear;
    state.perfect_clear = board_empty();
    if (state.perfect_clear) {
      state.score += 3500 * state.level;
    }
  } else {
    state.combo = -1;
    state.perfect_clear = false;
  }
  state.high_score = std::max(state.high_score, state.score);
}

int load_high_score() {
  std::ifstream file(state.high_score_path);
  int loaded = 0;
  if (file >> loaded) {
    return std::max(0, loaded);
  }
  return 0;
}

void save_high_score() {
  if (state.high_score_path.empty()) {
    return;
  }
  std::ofstream file(state.high_score_path, std::ios::trunc);
  if (file) {
    file << state.high_score << '\n';
  }
}

void spawn_next() {
  ensure_queue();
  state.current = make_piece(state.queue.front());
  state.queue.pop_front();
  ensure_queue();
  state.hold_used = false;
  if (collides(state.current)) {
    state.phase = Phase::GameOver;
  }
}

void lock_piece(int drop_bonus = 0) {
  const Color color = piece_color(state.current.type);
  for (const auto& block : blocks_for(state.current)) {
    if (block.row >= 0 && block.row < BOARD_HEIGHT && block.col >= 0 &&
        block.col < BOARD_WIDTH) {
      state.board[block.row][block.col] = Cell{true, color};
    }
  }
  const int cleared = clear_lines();
  apply_score(cleared, drop_bonus, is_t_spin());
  clear_lock_delay();
  state.last_action_rotate = false;
  spawn_next();
}

int fall_interval_value() { return std::max(100, 800 - (state.level - 1) * 60); }

void reset_game() {
  state.board = {};
  state.queue.clear();
  state.hold.reset();
  state.phase = Phase::Playing;
  state.score = 0;
  state.level = 1;
  state.lines = 0;
  state.combo = -1;
  state.back_to_back = false;
  state.perfect_clear = false;
  state.last_action_rotate = false;
  state.t_spin = false;
  state.hold_used = false;
  clear_lock_delay();
  state.high_score = std::max(state.high_score, load_high_score());
  running = true;
  ensure_queue();
  spawn_next();
  state.last_fall = std::chrono::steady_clock::now();
}
}  // namespace

std::atomic_bool running = false;

void init() {
  std::lock_guard<std::mutex> lock(state_mutex);
  state.high_score = load_high_score();
  reset_game();
}

void quit() {
  std::lock_guard<std::mutex> lock(state_mutex);
  save_high_score();
  running = false;
}

void set_high_score_path(const std::string& path) {
  std::lock_guard<std::mutex> lock(state_mutex);
  state.high_score_path = path;
  state.high_score = load_high_score();
}

void set_test_state(const Board& board, const Piece& piece) {
  std::lock_guard<std::mutex> lock(state_mutex);
  state.board = board;
  state.current = piece;
  state.phase = Phase::Playing;
  state.hold_used = false;
  state.last_action_rotate = false;
  state.t_spin = false;
}

void tick(std::chrono::steady_clock::time_point now) {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (state.phase != Phase::Playing) {
    state.last_fall = now;
    return;
  }
  if (now - state.last_fall < std::chrono::milliseconds(fall_interval_value())) {
    return;
  }
  if (!try_move(1, 0)) {
    if (!state.lock_started_at.has_value()) {
      state.lock_started_at = now;
    } else if (now - *state.lock_started_at >=
               std::chrono::milliseconds(LOCK_DELAY_MS)) {
      lock_piece();
    }
  } else {
    clear_lock_delay();
  }
  state.last_fall = now;
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
  if (state.phase == Phase::Playing) {
    if (try_move(0, -1)) {
      state.last_action_rotate = false;
      maybe_reset_lock_delay();
    }
  }
}

void right() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (state.phase == Phase::Playing) {
    if (try_move(0, 1)) {
      state.last_action_rotate = false;
      maybe_reset_lock_delay();
    }
  }
}

void down() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (state.phase != Phase::Playing) {
    return;
  }
  if (try_move(1, 0)) {
    state.score += 1;
    state.last_action_rotate = false;
    clear_lock_delay();
  } else {
    lock_piece();
  }
  state.last_fall = std::chrono::steady_clock::now();
}

void hard_drop() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (state.phase != Phase::Playing) {
    return;
  }
  int distance = 0;
  while (try_move(1, 0)) {
    distance++;
  }
  lock_piece(distance * 2);
  state.last_fall = std::chrono::steady_clock::now();
}

void hold() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (state.phase != Phase::Playing || state.hold_used) {
    return;
  }
  const PieceType previous = state.current.type;
  if (state.hold.has_value()) {
    state.current = make_piece(*state.hold);
    state.hold = previous;
    if (collides(state.current)) {
      state.phase = Phase::GameOver;
    }
  } else {
    state.hold = previous;
    spawn_next();
  }
  state.hold_used = true;
}

void toggle_pause() {
  std::lock_guard<std::mutex> lock(state_mutex);
  if (state.phase == Phase::Playing) {
    state.phase = Phase::Paused;
  } else if (state.phase == Phase::Paused) {
    state.phase = Phase::Playing;
    state.last_fall = std::chrono::steady_clock::now();
  }
}

void restart() {
  std::lock_guard<std::mutex> lock(state_mutex);
  reset_game();
}

Snapshot snapshot() {
  std::lock_guard<std::mutex> lock(state_mutex);
  return Snapshot{state.board, state.current, ghost_piece_unlocked(), state.queue,
                  state.hold,  state.phase, state.score,            state.level,
                  state.lines, fall_interval_value(), !state.hold_used,
                  state.high_score, state.combo, state.back_to_back,
                  state.perfect_clear, state.t_spin};
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
