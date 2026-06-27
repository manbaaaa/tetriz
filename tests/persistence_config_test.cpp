#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

#include "game.h"

namespace {
void assert_high_score_persists() {
  const std::string path = "tetriz_test_high_score";
  std::remove(path.c_str());

  gm::set_high_score_path(path);
  gm::init();
  gm::hard_drop();
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

void assert_high_score_table_loads_sorted_scores() {
  const std::string path = "tetriz_test_high_score_table";
  {
    std::ofstream file(path);
    file << "100\n500\n250\n50\n";
  }

  gm::set_high_score_path(path);
  gm::init();
  const gm::Snapshot snapshot = gm::snapshot();
  assert(snapshot.high_score == 500);
  assert(snapshot.high_scores[0].score == 500);
  assert(snapshot.high_scores[1].score == 250);
  assert(snapshot.high_scores[2].score == 100);
  assert(snapshot.high_scores[3].score == 50);
  assert(snapshot.high_scores[0].level == 1);
  gm::quit();
  std::remove(path.c_str());
}

void assert_high_score_table_loads_metadata() {
  const std::string path = "tetriz_test_high_score_metadata";
  {
    std::ofstream file(path);
    file << "700 8 42 123456\n";
  }

  gm::set_high_score_path(path);
  gm::init();
  const gm::Snapshot snapshot = gm::snapshot();
  assert(snapshot.high_scores[0].score == 700);
  assert(snapshot.high_scores[0].level == 8);
  assert(snapshot.high_scores[0].lines == 42);
  assert(snapshot.high_scores[0].played_at == 123456);
  gm::quit();
  std::remove(path.c_str());
}

void assert_start_level_config_applies_to_new_games() {
  gm::set_start_level(6);
  gm::init();
  assert(gm::snapshot().level == 6);
  gm::set_start_level(1);
}
}  // namespace

int main() {
  assert_high_score_persists();
  assert_high_score_table_loads_sorted_scores();
  assert_high_score_table_loads_metadata();
  assert_start_level_config_applies_to_new_games();
  gm::quit();
  return 0;
}
