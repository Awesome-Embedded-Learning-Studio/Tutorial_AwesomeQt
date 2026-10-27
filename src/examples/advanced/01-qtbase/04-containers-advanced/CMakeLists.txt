cmake_minimum_required(VERSION 3.26)                          # Qt6 要求的最低 CMake 版本
project(04-containers-advanced LANGUAGES CXX)                  # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                     # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                            # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                          # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                          # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                          # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core)                     # 本示例仅使用 Core 模块

qt_add_executable(${PROJECT_NAME}                              # 生成可执行文件
    cow_demo.h                                                 # COW 演示类声明
    cow_demo.cpp                                               # COW 演示类实现
    custom_hash.h                                              # 自定义哈希 key 声明
    custom_hash.cpp                                            # 自定义哈希 key 实现
    main.cpp                                                   # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                          # 链接 Core 模块
)
