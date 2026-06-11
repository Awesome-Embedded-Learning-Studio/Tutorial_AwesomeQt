cmake_minimum_required(VERSION 3.26)                            # Qt6 minimum CMake version
project(12-qtstatemachine-advanced LANGUAGES CXX)                # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                       # C++17 as required by project rules
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                            # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                            # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                            # Auto-handle .ui form files

find_package(Qt6 REQUIRED COMPONENTS Core StateMachine)          # QtStateMachine module

qt_add_executable(${PROJECT_NAME}                                # Build executable
    traffic_controller.h                                         # Header for MOC scanning
    traffic_controller.cpp                                       # State machine class impl
    main.cpp                                                     # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::StateMachine                          # Link required Qt modules
)

# Compiler warning flags for GCC/Clang and MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
