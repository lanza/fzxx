cmake_minimum_required(VERSION 3.12)
project(fzxx CXX)

add_library(fuzzy 
  src/choices.cpp
  src/options.cpp
  src/tty_interface.cpp
  src/match.cpp
  src/tty.cpp)

add_executable(fzxx src/fzy.cpp)
target_link_libraries(fzxx fuzzy)
