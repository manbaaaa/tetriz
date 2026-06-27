#include <cassert>
#include <chrono>
#include <cstdio>
#include <string>

#include "../control.h"
#include "../game.h"

namespace {
void assert_current_moved_left() {
  gm::init();
  const int col = gm::snapshot().current.col;
  gm::handle_command("\033[D");
  assert(gm::snapshot().current.col == col - 1);
}

void assert_current_rotates() {
  gm::Board board{};
  gm::set_test_state(board, gm::Piece{gm::PieceType::T, 4, 4, 0});
  const int rotation = gm::snapshot().current.rotation;
  gm::handle_command("w");
  assert(gm::snapshot().current.rotation == (rotation + 1) % 4);
  gm::handle_command("z");
  assert(gm::snapshot().current.rotation == rotation);
  gm::handle_command("x");
  assert(gm::snapshot().current.rotation == (rotation + 2) % 4);
}

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

void assert_hold_locks_until_next_piece() {
  gm::init();
  assert(gm::snapshot().hold_available);
  gm::handle_command("c");
  assert(!gm::snapshot().hold_available);
  while (!gm::snapshot().hold_available) {
    gm::handle_command(" ");
  }
  assert(gm::snapshot().hold_available);
}

void assert_scoring_feedback_defaults() {
  gm::init();
  assert(gm::snapshot().combo == -1);
  assert(!gm::snapshot().back_to_back);
  gm::handle_command(" ");
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
}

void assert_t_spin_detects_rotated_t_piece() {
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
  assert(snapshot.t_spin);
  assert(snapshot.score >= 400);
}

void assert_high_score_persists() {
  const std::string path = "tetriz_test_high_score";
  std::remove(path.c_str());

  gm::set_high_score_path(path);
  gm::init();
  gm::handle_command(" ");
  const int score = gm::snapshot().score;
  assert(score > 0);
  assert(gm::snapshot().high_score == score);
  gm::quit();

  gm::set_high_score_path(path);
  gm::init();
  assert(gm::snapshot().high_score == score);
  gm::quit();
  std::remove(path.c_str());
}
}  // namespace

int main() {
  assert_current_moved_left();
  assert_current_rotates();
  assert_srs_wall_kick_rotates_near_wall();
  assert_hold_locks_until_next_piece();
  assert_scoring_feedback_defaults();
  assert_lock_delay_defers_grounded_piece_lock();
  assert_perfect_clear_scores_bonus();
  assert_t_spin_detects_rotated_t_piece();
  assert_high_score_persists();
  gm::quit();
  return 0;
}
