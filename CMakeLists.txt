cmake_minimum_required(VERSION 3.12)
project(fzxx CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_library(fuzzy 
  src/choices.cpp
  src/options.cpp
  src/tty_interface.cpp
  src/match.cpp
  src/tty.cpp)

add_executable(fzxx src/fzy.cpp)
target_link_libraries(fzxx fuzzy)


include_directories(deps)
add_executable(fzxxtest 
  test/fzytest.cpp
  test/test_properties.cpp
  test/test_choices.cpp
  test/test_match.cpp
  deps/theft/theft.c
  deps/theft/theft_bloom.c
  deps/theft/theft_mt.c
  deps/theft/theft_hash.c
  )

target_link_libraries(fzxxtest fuzzy)
enable_testing()

add_test(TestFzxx fzxxtest)
