#include <cassert>
#include <chrono>

#include "game.h"

namespace {
void assert_srs_wall_kick_rotates_near_wall() {
  gm::Board board{};
  gm::Piece piece{gm::PieceType::I, 4, 0, 0};
  gm::set_test_state(board, piece);
  gm::rotate();
  const gm::Snapshot snapshot = gm::snapshot();
  assert(snapshot.current.rotation == 1);
  for (const auto& block : gm::blocks_for(snapshot.current)) {
    assert(block.col >= 0);
    assert(block.col < gm::BOARD_WIDTH);
  }
}

void assert_scoring_feedback_defaults() {
  gm::init();
  assert(gm::snapshot().combo == -1);
  assert(!gm::snapshot().back_to_back);
  gm::hard_drop();
  assert(gm::snapshot().combo == -1);
  assert(!gm::snapshot().back_to_back);
}

void assert_lock_delay_defers_grounded_piece_lock() {
  gm::Board board{};
  gm::Piece piece{gm::PieceType::O, gm::BOARD_HEIGHT - 2, 4, 0};
  gm::set_test_state(board, piece);

  const auto start = std::chrono::steady_clock::now();
  gm::tick(start + std::chrono::seconds(2));
  assert(gm::snapshot().current.type == gm::PieceType::O);
  assert(!gm::snapshot().board[gm::BOARD_HEIGHT - 1][4].filled);

  gm::tick(start + std::chrono::seconds(3));
  assert(gm::snapshot().board[gm::BOARD_HEIGHT - 1][4].filled);
}

void assert_perfect_clear_scores_bonus() {
  gm::Board board{};
  for (int col = 0; col < gm::BOARD_WIDTH; col++) {
    if (col >= 4 && col <= 7) {
      continue;
    }
    board[gm::BOARD_HEIGHT - 1][col] = gm::Cell{true, Color::BLUE};
  }

  gm::Piece piece{gm::PieceType::I, gm::BOARD_HEIGHT - 1, 5, 0};
  gm::set_test_state(board, piece);
  gm::hard_drop();
  const gm::Snapshot snapshot = gm::snapshot();
  assert(snapshot.perfect_clear);
  assert(snapshot.lines == 1);
  assert(snapshot.score >= 3600);
  assert(snapshot.recently_cleared_rows[gm::BOARD_HEIGHT - 1]);
}

void assert_t_spin_detects_rotated_t_piece() {
  gm::Board board{};
  board[5][3] = gm::Cell{true, Color::BLUE};
  board[5][5] = gm::Cell{true, Color::BLUE};
  board[7][3] = gm::Cell{true, Color::BLUE};
  board[7][5] = gm::Cell{true, Color::BLUE};
  board[8][3] = gm::Cell{true, Color::BLUE};
  board[8][4] = gm::Cell{true, Color::BLUE};
  board[8][5] = gm::Cell{true, Color::BLUE};

  gm::Piece piece{gm::PieceType::T, 6, 4, 0};
  gm::set_test_state(board, piece);
  gm::rotate();
  gm::hard_drop();
  const gm::Snapshot snapshot = gm::snapshot();
  assert(snapshot.t_spin == gm::TSpinType::Full);
  assert(snapshot.score >= 400);
}

void assert_t_spin_mini_detects_rotated_t_piece() {
  gm::Board board{};
  board[5][3] = gm::Cell{true, Color::BLUE};
  board[5][5] = gm::Cell{true, Color::BLUE};
  board[7][3] = gm::Cell{true, Color::BLUE};
  board[8][3] = gm::Cell{true, Color::BLUE};
  board[8][4] = gm::Cell{true, Color::BLUE};
  board[8][5] = gm::Cell{true, Color::BLUE};

  gm::Piece piece{gm::PieceType::T, 6, 4, 0};
  gm::set_test_state(board, piece);
  gm::rotate();
  gm::hard_drop();
  const gm::Snapshot snapshot = gm::snapshot();
  assert(snapshot.t_spin == gm::TSpinType::Mini);
  assert(snapshot.score >= 100);
}
}  // namespace

int main() {
  assert_srs_wall_kick_rotates_near_wall();
  assert_scoring_feedback_defaults();
  assert_lock_delay_defers_grounded_piece_lock();
  assert_perfect_clear_scores_bonus();
  assert_t_spin_detects_rotated_t_piece();
  assert_t_spin_mini_detects_rotated_t_piece();
  gm::quit();
  return 0;
}
