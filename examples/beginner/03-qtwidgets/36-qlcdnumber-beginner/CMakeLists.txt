# QtWidgets 入门示例 36: QLCDNumber 液晶数字显示
# display(int/double/QString) 三种显示方式
# setDigitCount 位数控制 + setSmallDecimalPoint 小数点策略
# setMode 十进制/十六进制/八进制/二进制切换
# QTimer + QLCDNumber 仪表盘计时器

cmake_minimum_required(VERSION 3.26)

project(36-qlcdnumber-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    lcddemowidget.cpp
    lcddemowidget.h
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
