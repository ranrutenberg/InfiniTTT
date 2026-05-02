cmake_minimum_required(VERSION 3.16)

project(InfiniTTT VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt6 automatic MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Core game library (shared between CLI and QML)
add_library(infinittt_core STATIC
    tictactoeboard.cpp
    src/ai/ai_utils.cpp
    src/ai/smart_random_ai.cpp
    src/ai/hybrid_evaluator_ai.cpp
    src/ai/hybrid_evaluator_ai_v2.cpp
    src/ai/hybrid_evaluator_ai_v3.cpp
)
target_include_directories(infinittt_core PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/ai
)

# CLI executable (desktop only)
if(NOT ANDROID)
    find_package(Threads REQUIRED)
    add_executable(InfiniTTT_CLI
        main.cpp
        weighttrainer.cpp
    )
    target_link_libraries(InfiniTTT_CLI PRIVATE infinittt_core Threads::Threads)
endif()

# QML GUI (Android and Desktop)
find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

set(QML_SOURCES
    src/qml/main_qml.cpp
    src/qml/boarditem.h
    src/qml/boarditem.cpp
    src/gui/gamecontroller.h
    src/gui/gamecontroller.cpp
    resources/weights.qrc
)

qt_add_executable(InfiniTTT_QML ${QML_SOURCES})

qt_add_resources(InfiniTTT_QML "qml_files"
    PREFIX "/"
    FILES
        qml/Main.qml
        qml/GameSetupDialog.qml
        qml/GameOverDialog.qml
        qml/DrawerItem.qml
)

target_link_libraries(InfiniTTT_QML PRIVATE
    infinittt_core
    Qt6::Quick
    Qt6::QuickControls2
)

target_include_directories(InfiniTTT_QML PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/ai
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/qml
)

# Android-specific settings
if(ANDROID)
    set_target_properties(InfiniTTT_QML PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_MIN_SDK_VERSION 26
        QT_ANDROID_TARGET_SDK_VERSION 36
        QT_ANDROID_VERSION_CODE 2
        QT_ANDROID_VERSION_NAME "1.0.0"
        QT_ANDROID_GRADLE_INIT_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/android/signing.gradle"
    )
else()
    if(WIN32)
        set_target_properties(InfiniTTT_QML PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
    if(APPLE)
        set_target_properties(InfiniTTT_QML PROPERTIES MACOSX_BUNDLE TRUE)
    endif()
endif()

# Install targets
if(NOT ANDROID)
    install(TARGETS InfiniTTT_CLI InfiniTTT_QML RUNTIME DESTINATION bin)
endif()
