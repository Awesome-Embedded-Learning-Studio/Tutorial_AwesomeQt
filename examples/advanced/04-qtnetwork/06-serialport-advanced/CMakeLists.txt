cmake_minimum_required(VERSION 3.26)                             # Qt6 required minimum
project(06-serialport-advanced LANGUAGES CXX)                     # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                        # C++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                             # Automatic MOC for Q_OBJECT classes
set(CMAKE_AUTORCC ON)                                             # Automatic resource compilation
set(CMAKE_AUTOUIC ON)                                             # Automatic UI compilation

find_package(Qt6 REQUIRED COMPONENTS Core SerialPort)             # Core + SerialPort modules

# Compiler warning flags for GCC and Clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)                 # Enable strict warnings
endif()

qt_add_executable(${PROJECT_NAME}                                 # Build executable
    protocol_parser.h                                              # Parser header (MOC scan)
    protocol_parser.cpp                                            # Parser implementation
    serial_manager.h                                               # Serial manager header (MOC scan)
    serial_manager.cpp                                             # Serial manager implementation
    main.cpp                                                       # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::SerialPort                              # Link required Qt modules
)
