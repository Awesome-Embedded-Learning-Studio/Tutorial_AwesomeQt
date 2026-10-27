cmake_minimum_required(VERSION 3.26)                           # Qt6 minimum CMake version
project(07-qtprintsupport-advanced LANGUAGES CXX)               # Project name and language

set(CMAKE_CXX_STANDARD 17)                                      # C++17 as required by project rules
set(CMAKE_CXX_STANDARD_REQUIRED ON)                             # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                           # Auto-run MOC for Q_OBJECT macros

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)     # Widgets for GUI, Gui for QPdfWriter

qt_add_executable(${PROJECT_NAME}                               # Build executable
    report_generator.h                                          # Header for MOC scanning
    report_generator.cpp                                        # Report generator implementation
    main.cpp                                                    # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                               # Core non-GUI functionality
        Qt6::Gui                                                # QPdfWriter, QPainter, QFont, etc.
        Qt6::Widgets                                            # QApplication, QWidget, etc.
)

# Compiler warning flags: strict warnings on GCC/Clang and MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
