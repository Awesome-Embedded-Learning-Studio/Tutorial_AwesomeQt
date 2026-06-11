cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(01-qtsql-advanced LANGUAGES CXX)                         # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                      # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                             # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                           # 自动运行 MOC，处理 Q_OBJECT

find_package(Qt6 REQUIRED COMPONENTS Core Sql)                  # 按需引入 Core 和 Sql 模块

qt_add_executable(${PROJECT_NAME}                               # 生成可执行文件
    connection_pool.h                                           # 连接池类声明
    connection_pool.cpp                                         # 连接池类实现
    transaction_demo.h                                          # 事务演示类声明
    transaction_demo.cpp                                        # 事务演示类实现
    main.cpp                                                    # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Sql                                  # 按需链接
)

# 编译器警告 flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
