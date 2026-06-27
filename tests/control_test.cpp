#include <cassert>

#include "control.h"
#include "game.h"

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

void assert_custom_key_binding_moves_piece() {
  gm::init();
  gm::configure_keys(gm::KeyBindings{
      'h',
  });
  const int col = gm::snapshot().current.col;
  gm::handle_command("h");
  assert(gm::snapshot().current.col == col - 1);
}

void assert_pending_commands_are_processed() {
  gm::init();
  gm::set_input_repeat_ms(0);
  const int col = gm::snapshot().current.col;
  gm::queue_command_for_test("\033[C");
  gm::process_pending_commands();
  assert(gm::snapshot().current.col == col + 1);
}

void assert_repeated_commands_are_throttled() {
  gm::init();
  gm::set_input_timing(1000, 1000);
  const int col = gm::snapshot().current.col;
  gm::queue_command_for_test("\033[C");
  gm::queue_command_for_test("\033[C");
  gm::process_pending_commands();
  assert(gm::snapshot().current.col == col + 1);
  gm::set_input_timing(170, 35);
}

void assert_rotation_repeat_has_separate_throttle() {
  gm::Board board{};
  gm::set_test_state(board, gm::Piece{gm::PieceType::T, 4, 4, 0});
  gm::set_input_timing(0, 0);
  gm::set_rotation_repeat_ms(1000);
  const int rotation = gm::snapshot().current.rotation;
  gm::queue_command_for_test("w");
  gm::queue_command_for_test("w");
  gm::process_pending_commands();
  assert(gm::snapshot().current.rotation == (rotation + 1) % 4);
  gm::set_rotation_repeat_ms(120);
}

void assert_visual_feedback_pulse_is_consumed() {
  gm::Board board{};
  gm::set_test_state(board, gm::Piece{gm::PieceType::T, 4, 4, 0});
  gm::set_visual_feedback_enabled(true);
  assert(!gm::consume_feedback_pulse());
  gm::handle_command("w");
  assert(gm::consume_feedback_pulse());
  assert(!gm::consume_feedback_pulse());
  gm::set_visual_feedback_enabled(false);
}
}  // namespace

int main() {
  assert_current_moved_left();
  assert_current_rotates();
  assert_hold_locks_until_next_piece();
  assert_custom_key_binding_moves_piece();
  assert_pending_commands_are_processed();
  assert_repeated_commands_are_throttled();
  assert_rotation_repeat_has_separate_throttle();
  assert_visual_feedback_pulse_is_consumed();
  gm::quit();
  return 0;
}
