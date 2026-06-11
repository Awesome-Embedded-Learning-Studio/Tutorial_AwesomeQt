cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(04-websocket-advanced LANGUAGES CXX)                      # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                        # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                             # 自动运行 MOC，处理 Q_OBJECT

# 编译器警告标志，GCC 和 Clang 通用
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)             # 查找 Qt6 Core 和 WebSockets 模块

qt_add_executable(${PROJECT_NAME}                                 # 生成可执行文件
    reconnect_client.h                                            # 重连客户端头文件（MOC 扫描用）
    reconnect_client.cpp                                          # 重连客户端实现
    echo_server.h                                                 # Echo 服务器头文件（MOC 扫描用）
    echo_server.cpp                                               # Echo 服务器实现
    main.cpp                                                      # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::WebSockets                             # 链接 Qt6 Core 和 WebSockets
)
