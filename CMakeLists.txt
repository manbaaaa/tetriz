cmake_minimum_required(VERSION 3.22)
project(tetriz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(tetriz
  control.cpp
  draw.cpp
  game.cpp
  main.cpp
  terminal.cpp
  utils.cpp
)
target_link_libraries(tetriz PRIVATE Threads::Threads)

enable_testing()

add_executable(tetriz_game_control_test
  control.cpp
  game.cpp
  tests/game_control_test.cpp
)
target_link_libraries(tetriz_game_control_test PRIVATE Threads::Threads)
add_test(NAME tetriz_game_control_test COMMAND tetriz_game_control_test)
