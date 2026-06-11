cmake_minimum_required(VERSION 3.26)                          # Qt6 要求的最低 CMake 版本
project(03-http-advanced LANGUAGES CXX)                        # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                    # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                           # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                         # 自动运行 MOC，处理 Q_OBJECT

# 编译器警告标志：尽早发现问题
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network)            # 引入 Core 和 Network 模块

qt_add_executable(${PROJECT_NAME}                             # 生成可执行文件
    request_manager.h                                         # 请求管理器头文件（MOC 扫描用）
    request_manager.cpp                                       # 请求管理器实现
    cookie_persistence.h                                      # Cookie 持久化头文件
    cookie_persistence.cpp                                    # Cookie 持久化实现
    main.cpp                                                  # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Network                            # 链接 Core 和 Network
)
