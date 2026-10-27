cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(04-cpp-qml-interop-advanced LANGUAGES CXX)               # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                             # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                             # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                             # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)         # 按需添加模块

# 编译器警告标志：GCC/Clang 和 MSVC 分别设置
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)                 # GCC/Clang 最高警告级别
elseif(MSVC)
    add_compile_options(/W4)                                      # MSVC 4 级警告
endif()

qt_add_executable(${PROJECT_NAME}                                 # 生成可执行文件
    task_model.h                                                  # 任务模型头文件（MOC 扫描用）
    task_model.cpp                                                # 任务模型实现
    app_controller.h                                              # 控制器头文件
    app_controller.cpp                                            # 控制器实现
    main.cpp                                                      # 程序入口
)

# 将 QML 文件打包为 QML 模块，URI 与 import 语句对应
qt6_add_qml_module(${PROJECT_NAME}
    URI CppQmlInteropAdvanced                                     # QML import 名称
    QML_FILES
        main.qml                                                  # 主界面 QML 文件
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml               # 按需链接
)
