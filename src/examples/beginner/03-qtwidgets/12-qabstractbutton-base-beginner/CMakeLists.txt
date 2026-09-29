# QtWidgets 入门示例 12: QAbstractButton 按钮基类机制
# 核心属性：setCheckable / setChecked / setAutoRepeat
# 四个信号：clicked / toggled / pressed / released
# QButtonGroup 单选互斥管理
# 继承 QAbstractButton 自定义按钮

cmake_minimum_required(VERSION 3.26)

project(12-qabstractbutton-base-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    CircleButton.h
    CircleButton.cpp
    DemoWindow.h
    DemoWindow.cpp
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
