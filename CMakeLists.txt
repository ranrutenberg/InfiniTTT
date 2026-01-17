cmake_minimum_required(VERSION 3.10)

project(InfiniTTT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add src/ai as an include directory
include_directories(${CMAKE_SOURCE_DIR})

add_executable(InfiniTTT
    main.cpp
    tictactoeboard.cpp
    weighttrainer.cpp
    src/ai/ai_utils.cpp
    src/ai/smart_random_ai.cpp
    src/ai/hybrid_evaluator_ai.cpp
)

install(TARGETS InfiniTTT RUNTIME DESTINATION bin)
