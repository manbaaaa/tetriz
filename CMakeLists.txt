cmake_minimum_required(VERSION 3.22)
project(tetriz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(tetriz_config
  config.cpp
)
target_include_directories(tetriz_config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(tetriz_core
  game.cpp
)
target_include_directories(tetriz_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(tetriz_controls
  control.cpp
)
target_include_directories(tetriz_controls PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tetriz_controls PUBLIC tetriz_core Threads::Threads)

add_executable(tetriz
  draw.cpp
  main.cpp
  terminal.cpp
  utils.cpp
)
target_link_libraries(tetriz PRIVATE tetriz_controls tetriz_config Threads::Threads)

enable_testing()

function(add_tetriz_test target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} PRIVATE tetriz_controls)
  add_test(NAME ${target} COMMAND ${target})
endfunction()

add_tetriz_test(tetriz_control_test tests/control_test.cpp)
add_tetriz_test(tetriz_rules_test tests/rules_test.cpp)
add_tetriz_test(tetriz_persistence_config_test tests/persistence_config_test.cpp)

add_executable(tetriz_config_test tests/config_test.cpp)
target_link_libraries(tetriz_config_test PRIVATE tetriz_config)
add_test(NAME tetriz_config_test COMMAND tetriz_config_test)
