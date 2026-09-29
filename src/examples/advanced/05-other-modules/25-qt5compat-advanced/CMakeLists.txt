cmake_minimum_required(VERSION 3.26)                          # Qt6 要求的最低 CMake 版本
project(25-qt5compat-advanced LANGUAGES CXX)                    # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                      # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                             # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                           # 自动运行 MOC，处理 Q_OBJECT

find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat)          # Core5Compat 提供 Qt5 兼容 API

qt_add_executable(${PROJECT_NAME}                               # 生成可执行文件
    migration_demo.h                                            # 演示类声明（MOC 扫描用）
    migration_demo.cpp                                          # 演示类实现
    main.cpp                                                    # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Core5Compat                          # 链接 Core 和 Core5Compat 模块
)

# 编译器警告 flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)  # GCC/Clang 最高警告级别
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)         # MSVC /W4 级别警告
endif()
