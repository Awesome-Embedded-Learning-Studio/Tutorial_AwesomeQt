cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(05-qvariant-metatype-advanced LANGUAGES CXX)             # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                            # 自动运行 MOC
set(CMAKE_AUTORCC ON)                                            # 自动处理 .qrc
set(CMAKE_AUTOUIC ON)                                            # 自动处理 .ui

find_package(Qt6 REQUIRED COMPONENTS Core)                       # 按需添加模块

qt_add_executable(${PROJECT_NAME}                                # 生成可执行文件
    main.cpp                                                     # 程序入口
    custom_type.h                                                # 自定义传感器类型声明
    custom_type.cpp                                              # 自定义类型实现与演示
    metatype_reflection.h                                        # QMetaType 反射演示声明
    metatype_reflection.cpp                                      # QMetaType 反射演示实现
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                             # 按需链接
)
