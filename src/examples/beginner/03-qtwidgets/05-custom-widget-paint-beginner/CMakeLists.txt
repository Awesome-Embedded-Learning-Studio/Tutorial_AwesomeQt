# QtWidgets 入门示例 05: 自定义绘制 Widget 基础
# paintEvent / update vs repaint / 双缓冲 / sizeHint + minimumSizeHint
# 圆形仪表盘控件 + QPropertyAnimation 平滑动画

cmake_minimum_required(VERSION 3.26)

project(05-custom-widget-paint-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    circulargauge.h
    circulargauge.cpp
    mainwindow.h
    mainwindow.cpp
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
