#include <cassert>
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
  gm::init();
  const int rotation = gm::snapshot().current.rotation;
  gm::handle_command("w");
  assert(gm::snapshot().current.rotation == (rotation + 1) % 4);
  gm::handle_command("z");
  assert(gm::snapshot().current.rotation == rotation);
  gm::handle_command("x");
  assert(gm::snapshot().current.rotation == (rotation + 2) % 4);
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
  assert_hold_locks_until_next_piece();
  assert_scoring_feedback_defaults();
  assert_high_score_persists();
  gm::quit();
  return 0;
}
