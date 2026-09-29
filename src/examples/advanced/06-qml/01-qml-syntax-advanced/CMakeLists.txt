cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(01-qml-syntax-advanced LANGUAGES CXX)                    # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                            # 自动运行 MOC，处理 Q_OBJECT

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)         # QML 应用所需的模块

qt_add_executable(${PROJECT_NAME}                                # 生成可执行文件
    main.cpp                                                     # 程序入口，启动 QML 引擎
)

# 注册 QML 模块：URI 为 QmlSyntaxAdvanced，包含 main.qml
# qt6_add_qml_module 会自动生成 qrc 并注册到 C++ 构建
qt6_add_qml_module(${PROJECT_NAME}
    URI QmlSyntaxAdvanced
    QML_FILES
        main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                                # 核心非 GUI 功能
        Qt6::Gui                                                 # 窗口系统集成
        Qt6::Quick                                               # QML 视图框架
        Qt6::Qml                                                 # QML 引擎
)

# GCC/Clang 编译器警告标志
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME}
        PRIVATE -Wall -Wextra -Wpedantic)
endif()

# MSVC 编译器警告标志
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
