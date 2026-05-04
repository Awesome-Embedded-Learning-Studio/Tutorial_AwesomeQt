# QtWidgets 入门示例 14: QAbstractScrollArea 滚动区域基类
# horizontalScrollBar() / verticalScrollBar() 滚动条控制
# setHorizontalScrollBarPolicy / setVerticalScrollBarPolicy
# scrollContentsBy() 内容滚动响应
# 视口（viewport）概念与绘制

cmake_minimum_required(VERSION 3.26)

project(14-qabstractscrollarea-base-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    demo_window.h
    demo_window.cpp
    timeline_scroll_area.h
    timeline_scroll_area.cpp
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
