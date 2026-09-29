cmake_minimum_required(VERSION 3.26)                                 # Qt6 minimum CMake version
project(23-qtspatial-audio-advanced LANGUAGES CXX)                    # Project name and language

set(CMAKE_CXX_STANDARD 17)                                           # Use C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                  # Do not fall back to older standards
set(CMAKE_AUTOMOC ON)                                                # Auto-run MOC for Q_OBJECT macros

find_package(Qt6 REQUIRED COMPONENTS Core SpatialAudio)              # Core + Spatial Audio modules

qt_add_executable(${PROJECT_NAME}                                    # Qt-aware executable target
    audio_scene.h                                                    #   Header for MOC scanning
    audio_scene.cpp                                                  #   AudioScene implementation
    main.cpp                                                         #   Program entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::SpatialAudio                              # Link required Qt modules
)

# Compiler warning flags — GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Compiler warning flags — MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
