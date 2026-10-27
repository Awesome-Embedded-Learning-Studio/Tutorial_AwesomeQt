cmake_minimum_required(VERSION 3.26)                          # Qt6 minimum CMake version
project(06-qtsvg-advanced LANGUAGES CXX)                       # Project name and language

set(CMAKE_CXX_STANDARD 17)                                     # Use C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                            # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                          # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                          # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                          # Auto-handle .ui form files

find_package(Qt6 REQUIRED COMPONENTS Core Xml Svg)             # Core + XML for DOM + Svg module

qt_add_executable(${PROJECT_NAME}                              # Build executable
    svg_controller.h                                            # SvgController declaration
    svg_controller.cpp                                          # SvgController implementation
    main.cpp                                                    # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Xml Qt6::Svg                        # Link required Qt modules
)

# Compiler warning flags — strict warnings on all platforms
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
