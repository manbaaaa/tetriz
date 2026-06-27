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

#include <array>
#include <atomic>
#include <chrono>
#include <deque>
#include <optional>
#include <string>

#include "./color.h"
#include "./define.h"

namespace gm {
constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 20;
constexpr int PREVIEW_COUNT = 3;
constexpr int HIGH_SCORE_COUNT = 5;

enum class PieceType { I, J, L, O, S, T, Z };
enum class Phase { Playing, Paused, GameOver };
enum class TSpinType { None, Mini, Full };

struct Cell {
  bool filled = false;
  Color color = Color::BLACK;
};

struct Point {
  int row = 0;
  int col = 0;
};

struct Piece {
  PieceType type = PieceType::I;
  int row = 0;
  int col = 0;
  int rotation = 0;
};

struct HighScoreEntry {
  int score = 0;
  int level = 1;
  int lines = 0;
  long long played_at = 0;
};

using Board = std::array<std::array<Cell, BOARD_WIDTH>, BOARD_HEIGHT>;
using Blocks = std::array<Point, 4>;

struct Snapshot {
  Board board;
  Piece current;
  Piece ghost;
  std::deque<PieceType> next;
  std::optional<PieceType> hold;
  Phase phase = Phase::Playing;
  int score = 0;
  int level = 1;
  int lines = 0;
  int fall_interval_ms = 800;
  bool hold_available = true;
  int high_score = 0;
  std::array<HighScoreEntry, HIGH_SCORE_COUNT> high_scores = {};
  int combo = -1;
  bool back_to_back = false;
  bool perfect_clear = false;
  TSpinType t_spin = TSpinType::None;
  std::array<bool, BOARD_HEIGHT> recently_cleared_rows = {};
};

extern std::atomic_bool running;

void init();
void quit();
void tick(std::chrono::steady_clock::time_point now);
void set_high_score_path(const std::string& path);
void set_start_level(int level);
void set_test_state(const Board& board, const Piece& piece);

bool rotate();
bool rotate_counterclockwise();
bool rotate_180();
bool left();
bool right();
bool down();
bool hard_drop();
bool hold();
bool toggle_pause();
bool restart();

Snapshot snapshot();
Blocks blocks_for(const Piece& piece);
Color piece_color(PieceType type);
char piece_name(PieceType type);
}  // namespace gm
