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

#include "./color.h"
#include "./define.h"

namespace gm {
constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 20;
constexpr int PREVIEW_COUNT = 3;

enum class PieceType { I, J, L, O, S, T, Z };
enum class Phase { Playing, Paused, GameOver };

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
};

extern std::atomic_bool running;

void init();
void quit();
void tick(std::chrono::steady_clock::time_point now);

void rotate();
void left();
void right();
void down();
void hard_drop();
void hold();
void toggle_pause();
void restart();

Snapshot snapshot();
Blocks blocks_for(const Piece& piece);
Color piece_color(PieceType type);
char piece_name(PieceType type);
}  // namespace gm
