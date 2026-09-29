cmake_minimum_required(VERSION 3.26)                                # Qt6 要求的最低 CMake 版本
project(02-qtsql-tablemodel-advanced LANGUAGES CXX)                  # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                           # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                  # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                                # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                                # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                                # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets)              # 按需添加模块

qt_add_executable(${PROJECT_NAME}                                    # 生成可执行文件
    relational_model.h                                                # RelationalModelSetup 类声明（MOC 扫描用）
    relational_model.cpp                                              # RelationalModelSetup 类实现
    main.cpp                                                          # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Sql Qt6::Widgets                          # 按需链接
)

# GCC / Clang 编译警告
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# MSVC 编译警告
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
