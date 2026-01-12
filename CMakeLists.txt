cmake_minimum_required(VERSION 3.0)

project(tictactoe)

add_executable(tictactoe main.cpp tictactoeboard.cpp aiplayer.cpp)

install(TARGETS tictactoe RUNTIME DESTINATION bin)
