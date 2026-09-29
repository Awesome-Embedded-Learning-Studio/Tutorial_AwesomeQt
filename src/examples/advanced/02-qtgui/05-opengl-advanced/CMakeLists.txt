cmake_minimum_required(VERSION 3.26)                        # Qt6 要求的最低 CMake 版本
project(05-opengl-advanced LANGUAGES CXX)                    # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                   # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                          # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                        # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                        # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                        # 自动处理 .ui 文件（如有）

# OpenGLWidgets 模块从 Qt 6.x 起提供 QOpenGLWidget 等类
# 优先尝试查找完整组件集；如果 OpenGLWidgets 不可用则退化为仅 Core Gui Widgets
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

if(Qt6OpenGLWidgets_FOUND)
    set(QT_OPENGL_LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets)
    message(STATUS "Using Qt6OpenGLWidgets")
else()
    # 回退：部分 Qt 安装可能不包含 OpenGLWidgets 模块
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL)
    set(QT_OPENGL_LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL)
    message(STATUS "Qt6OpenGLWidgets not found, falling back to Core Gui Widgets OpenGL")
endif()

qt_add_executable(${PROJECT_NAME}                            # 生成可执行文件
    gl_widget.h                                              # 头文件（MOC 扫描用）
    gl_widget.cpp                                            # 类实现
    main.cpp                                                 # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE ${QT_OPENGL_LIBS}                                # 链接所需模块
)
