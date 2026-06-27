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

#include "./control.h"
#include "./draw.h"
#include "./game.h"
#include "./terminal.h"
#include "./utils.h"

#include <csignal>
#include <cstdlib>

namespace {
class TerminalGuard {
 public:
  TerminalGuard() { tc::hide_cursor(); }
  ~TerminalGuard() { restore_terminal(); }

  TerminalGuard(const TerminalGuard&) = delete;
  TerminalGuard& operator=(const TerminalGuard&) = delete;

 private:
  void restore_terminal() {
    tc::show_cursor();
    tc::reset_color();
    tc::clear_screen();
    tc::move_to(1, 1);
  }
};

void handle_signal(int) {
  gm::quit();
}
}  // namespace

void init() {
  if (const char* high_score_path = std::getenv("TETRIZ_HIGH_SCORE_PATH")) {
    gm::set_high_score_path(high_score_path);
  }
  gm::init();
  gm::start_listener();
}

void loop() {
  auto last_render = std::chrono::steady_clock::time_point{};
  while (gm::running.load()) {
    const auto now = std::chrono::steady_clock::now();
    gm::tick(now);
    if (now - last_render >= std::chrono::milliseconds(100)) {
      dw::render(ut::fps());
      last_render = now;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

int main() {
  std::signal(SIGINT, handle_signal);
  std::signal(SIGTERM, handle_signal);
  {
    TerminalGuard terminal_guard;
    init();
    loop();
  }
  std::cout << "Bye!" << std::endl;
  return 0;
}
