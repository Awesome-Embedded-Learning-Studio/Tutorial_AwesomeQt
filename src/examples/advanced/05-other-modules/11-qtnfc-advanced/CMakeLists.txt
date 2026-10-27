cmake_minimum_required(VERSION 3.26)                            # Qt6 minimum CMake version
project(11-qtnfc-advanced LANGUAGES CXX)                         # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                       # C++17 as per coding rules
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                            # Auto-run MOC for Q_OBJECT macros

find_package(Qt6 REQUIRED COMPONENTS Core Nfc)                   # Core + QtNfc module

qt_add_executable(${PROJECT_NAME}                                # Build executable
    ndef_builder.h                                               # Builder class header (MOC scans this)
    ndef_builder.cpp                                             # Builder class implementation
    main.cpp                                                     # Console entry point with 5 demos
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Nfc                                   # Link against Core and Nfc
)

# Compiler warning flags — strict warnings on all platforms
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic)                               # GCC/Clang: full warning suite
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)          # MSVC: level 4 warnings
endif()
