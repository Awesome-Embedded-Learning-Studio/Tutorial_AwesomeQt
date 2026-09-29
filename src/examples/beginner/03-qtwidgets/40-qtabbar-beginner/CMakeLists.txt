# QtWidgets 入门示例 40: QTabBar 独立标签栏
# QTabBar 与 QTabWidget 的区别（可独立使用）
# 自定义标签栏 + 自定义内容区域组合
# tabCloseRequested 信号实现可关闭标签页
# setMovable(true) 可拖动标签排序

cmake_minimum_required(VERSION 3.26)

project(40-qtabbar-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    TabBarDemoWidget.h
    TabBarDemoWidget.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets
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
