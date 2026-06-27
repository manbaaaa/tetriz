#include <cassert>
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
}  // namespace

int main() {
  assert_current_moved_left();
  assert_current_rotates();
  assert_hold_locks_until_next_piece();
  gm::quit();
  return 0;
}
