# QtGui 入门示例 05: QOpenGLWidget 嵌入 OpenGL 基础
# OpenGL 三角形渲染示例，需要 Widgets、GUI 和 OpenGL 相关模块

cmake_minimum_required(VERSION 3.26)

project(05-opengl-widget-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets OpenGLWidgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.h
    mainwindow.cpp
    triangleglwidget.h
    triangleglwidget.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
