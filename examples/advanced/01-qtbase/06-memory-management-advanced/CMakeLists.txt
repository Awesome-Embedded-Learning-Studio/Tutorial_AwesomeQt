cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(06-memory-management-advanced LANGUAGES CXX)              # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                            # 自动运行 MOC
set(CMAKE_AUTORCC ON)                                            # 自动处理 .qrc
set(CMAKE_AUTOUIC ON)                                            # 自动处理 .ui

find_package(Qt6 REQUIRED COMPONENTS Core)                       # 按需添加模块

qt_add_executable(${PROJECT_NAME}                                # 生成可执行文件
    smart_pointer_demo.h                                         # 智能指针演示 - 头文件
    smart_pointer_demo.cpp                                       # 智能指针演示 - 实现
    circular_fix.h                                               # 循环引用修复 - 头文件
    circular_fix.cpp                                             # 循环引用修复 - 实现
    qpointer_demo.h                                              # QPointer 演示 - 头文件
    qpointer_demo.cpp                                            # QPointer 演示 - 实现
    main.cpp                                                     # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                             # 按需链接
)
