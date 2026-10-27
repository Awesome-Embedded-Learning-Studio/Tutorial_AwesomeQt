# QtWidgets 入门示例 11: QWidget 基类基础
# 窗口属性：resize / move / setWindowTitle / setWindowIcon
# show() / hide() / setVisible() / raise() / lower()
# 尺寸策略：setSizePolicy / setFixedSize / setMinimumSize
# 窗口标志 Qt::WindowFlags（无边框、置顶、工具窗口）

cmake_minimum_required(VERSION 3.26)

project(11-qwidget-base-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    DraggableWidget.h
    DraggableWidget.cpp
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
