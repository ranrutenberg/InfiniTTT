cmake_minimum_required(VERSION 3.0)

project(tictactoe)

add_executable(tictactoe main.cpp)

install(TARGETS tictactoe RUNTIME DESTINATION bin)
