cmake_minimum_required(VERSION 3.26)                           # Qt6 minimum CMake version
project(24-qttexttospeech-advanced LANGUAGES CXX)               # Project name and language

set(CMAKE_CXX_STANDARD 17)                                      # C++17 as required by project rules
set(CMAKE_CXX_STANDARD_REQUIRED ON)                             # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                           # Auto-run MOC for Q_OBJECT classes
set(CMAKE_AUTORCC ON)                                           # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                           # Auto-handle .ui form files

find_package(Qt6 REQUIRED COMPONENTS Core TextToSpeech)         # Core + TextToSpeech modules

# Compiler warning flags — strict warnings on all platforms
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    add_compile_options(/W4)
endif()

qt_add_executable(${PROJECT_NAME}                               # Qt-aware executable target
    speech_manager.h                                             # Header (MOC scans for Q_OBJECT)
    speech_manager.cpp                                           # SpeechManager implementation
    main.cpp                                                     # Program entry point & demos
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                               # Core non-GUI functionality
        Qt6::TextToSpeech                                       # Text-to-speech engine API
)
