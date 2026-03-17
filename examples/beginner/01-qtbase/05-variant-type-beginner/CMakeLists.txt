cmake_minimum_required(VERSION 3.26)                    # Qt 6 要求的最低 CMake 版本
project(05-variant-type-beginner LANGUAGES CXX)           # 项目名与语言

set(CMAKE_CXX_STANDARD 23)                               # 使用 C++23 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)                      # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                    # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                    # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                    # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core Gui)            # 查找 Qt 6 Core 和 Gui 模块（QColor 需要 Gui）

qt_add_executable(${PROJECT_NAME}
    main.cpp                                             # 示例源代码
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui                           # 链接 Qt 6 Core 和 Gui
)
