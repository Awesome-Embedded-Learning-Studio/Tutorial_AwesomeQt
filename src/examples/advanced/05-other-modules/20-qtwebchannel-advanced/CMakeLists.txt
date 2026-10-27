cmake_minimum_required(VERSION 3.26)                            # Qt6 required minimum
project(20-qtwebchannel-advanced LANGUAGES CXX)                  # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                       # C++17 as per project standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                            # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                            # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                            # Auto-handle .ui form files

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets WebChannel) # Three modules used in this example

qt_add_executable(${PROJECT_NAME}                                # Build executable with all source files
    chat_backend.h                                                # Backend class declaration (MOC scan)
    chat_backend.cpp                                              # Backend class implementation
    channel_server.h                                              # Server + transport class declarations
    channel_server.cpp                                            # Server + transport class implementations
    test_client.h                                                 # Test client class declaration (MOC scan)
    test_client.cpp                                               # Test client class implementation
    main.cpp                                                      # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::WebSockets Qt6::WebChannel             # Link required Qt modules
)

# Compiler warning flags for GCC/Clang and MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
