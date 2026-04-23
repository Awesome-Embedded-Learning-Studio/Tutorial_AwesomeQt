# QtGui 入门示例 03: QImage、QPixmap、QIcon 图像处理基础
# 图像加载、缩放、QIcon 动态生成示例，需要 Widgets 和 GUI 模块

cmake_minimum_required(VERSION 3.26)

project(03-image-pixmap-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    pixeldemowidget.h
    pixeldemowidget.cpp
    imagedisplaywidget.h
    imagedisplaywidget.cpp
    simpleimageviewer.h
    simpleimageviewer.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Gui Qt6::Widgets
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
