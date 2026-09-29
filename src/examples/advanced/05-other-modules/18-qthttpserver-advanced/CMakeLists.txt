cmake_minimum_required(VERSION 3.26)                            # Qt6 minimum CMake version
project(18-qthttpserver-advanced LANGUAGES CXX)                  # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                       # Unified C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                            # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                            # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                            # Auto-handle .ui form files

find_package(Qt6 REQUIRED COMPONENTS Core Network HttpServer)    # Qt modules used in this example

# ── Compiler warning flags ────────────────────────────────────────────────────
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)                # GCC/Clang strict warnings
elseif(MSVC)
    add_compile_options(/W4)                                     # MSVC high warning level
endif()

qt_add_executable(${PROJECT_NAME}                                # Qt-aware executable target
    api_server.h                                                  # Server class declaration
    api_server.cpp                                                # Server class implementation
    auth_middleware.h                                              # Auth middleware declaration
    auth_middleware.cpp                                            # Auth middleware implementation
    main.cpp                                                      # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                                 # Core non-GUI classes
        Qt6::Network                                              # QNetworkAccessManager for self-test
        Qt6::HttpServer                                           # QHttpServer for HTTP routing
)
