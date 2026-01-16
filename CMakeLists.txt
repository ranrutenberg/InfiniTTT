cmake_minimum_required(VERSION 3.0)

project(InfiniTTT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(InfiniTTT main.cpp tictactoeboard.cpp aiplayer.cpp weighttrainer.cpp)

install(TARGETS InfiniTTT RUNTIME DESTINATION bin)
