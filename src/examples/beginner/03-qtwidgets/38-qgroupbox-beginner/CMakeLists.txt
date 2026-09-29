# QtWidgets 入门示例 38: QGroupBox 分组框
# 带标题边框的控件分组容器
# setCheckable(true) 可勾选分组框（整组启用/禁用）
# setAlignment 标题对齐
# 嵌套布局的正确姿势

cmake_minimum_required(VERSION 3.26)

project(38-qgroupbox-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    GroupBoxDemoWidget.cpp
    GroupBoxDemoWidget.h
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
