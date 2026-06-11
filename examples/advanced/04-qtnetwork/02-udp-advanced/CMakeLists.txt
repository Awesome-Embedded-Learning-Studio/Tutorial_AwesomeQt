cmake_minimum_required(VERSION 3.26)                              # Qt6 要求的最低 CMake 版本
project(02-udp-advanced LANGUAGES CXX)                             # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                        # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                             # 自动运行 MOC，处理 Q_OBJECT

# 编译器警告标志：GCC/Clang 下启用全部警告和 pedantic 检查
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network)                # 仅引入本示例实际用到的模块

qt_add_executable(${PROJECT_NAME}                                 # 生成可执行文件
    multicast_manager.h                                            # 组播管理器头文件（MOC 扫描用）
    multicast_manager.cpp                                          # 组播管理器实现
    fragment_handler.h                                             # 分片处理器头文件（MOC 扫描用）
    fragment_handler.cpp                                           # 分片处理器实现
    main.cpp                                                       # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Network                                 # 按需链接
)
