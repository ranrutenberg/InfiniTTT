cmake_minimum_required(VERSION 3.16)

project(InfiniTTT VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt6 automatic MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Core game library (shared between CLI and GUI)
add_library(infinittt_core STATIC
    tictactoeboard.cpp
    src/ai/ai_utils.cpp
    src/ai/smart_random_ai.cpp
    src/ai/hybrid_evaluator_ai.cpp
    src/ai/hybrid_evaluator_ai_v2.cpp
)
target_include_directories(infinittt_core PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/ai
)

# CLI executable
add_executable(InfiniTTT_CLI
    main.cpp
    weighttrainer.cpp
)
target_link_libraries(InfiniTTT_CLI PRIVATE infinittt_core)

# GUI executable
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(GUI_SOURCES
    src/gui/main_gui.cpp
    src/gui/mainwindow.cpp
    src/gui/gamesetupdialog.cpp
    src/gui/configdialog.cpp
    src/gui/boardview.cpp
    src/gui/gridgraphicsitem.cpp
    src/gui/gamecontroller.cpp
)

set(GUI_HEADERS
    src/gui/mainwindow.h
    src/gui/gamesetupdialog.h
    src/gui/configdialog.h
    src/gui/boardview.h
    src/gui/gridgraphicsitem.h
    src/gui/gamecontroller.h
)

add_executable(InfiniTTT_GUI
    ${GUI_SOURCES}
    ${GUI_HEADERS}
)

target_link_libraries(InfiniTTT_GUI PRIVATE
    infinittt_core
    Qt6::Widgets
)

# Platform-specific settings
if(WIN32)
    set_target_properties(InfiniTTT_GUI PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(APPLE)
    set_target_properties(InfiniTTT_GUI PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

# Install targets
install(TARGETS InfiniTTT_CLI InfiniTTT_GUI RUNTIME DESTINATION bin)
