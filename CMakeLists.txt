cmake_minimum_required(VERSION 3.28)

project(tictactoe)

# Enable C++20 and module support
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP ON)

# Add module interface files
add_executable(tictactoe
    main.cpp
)

target_sources(tictactoe
    PUBLIC
        FILE_SET CXX_MODULES FILES
            tictactoeboard.cppm
            aiplayer.cppm
)

install(TARGETS tictactoe RUNTIME DESTINATION bin)
