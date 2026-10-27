# QtGui 入门示例 04: QFont 与文本渲染基础
# 文字排版、QFontMetrics 计算、QTextDocument 富文本示例，需要 Widgets 和 GUI 模块

cmake_minimum_required(VERSION 3.26)

project(04-font-text-rendering-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    fontdemowidget.h
    fontdemowidget.cpp
    metricslayoutwidget.h
    metricslayoutwidget.cpp
    richtextcardwidget.h
    richtextcardwidget.cpp
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
