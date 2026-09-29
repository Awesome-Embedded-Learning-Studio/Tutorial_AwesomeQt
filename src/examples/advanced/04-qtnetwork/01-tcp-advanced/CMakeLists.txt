cmake_minimum_required(VERSION 3.26)                    # Qt6 minimum CMake version
project(01-tcp-advanced LANGUAGES CXX)                   # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                               # Unified C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                      # No fallback to older standards

set(CMAKE_AUTOMOC ON)                                    # Automatic MOC for Q_OBJECT macros

find_package(Qt6 REQUIRED COMPONENTS Core Network)       # Core event loop + TCP sockets

# Compiler warning flags — GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Compiler warning flags — MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
endif()

qt_add_executable(${PROJECT_NAME}                        # Build the demo executable
    protocol_frame.h                                     # Frame encoder/parser header
    protocol_frame.cpp                                   # Frame encoder/parser implementation
    tcp_server.h                                         # Multi-client TCP server header
    tcp_server.cpp                                       # Multi-client TCP server implementation
    main.cpp                                             # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Network                       # Link against Qt Core and Network
)
