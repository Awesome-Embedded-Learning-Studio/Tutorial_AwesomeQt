# QtWidgets 入门示例 42: QSplitter 可拖动分割容器
# 水平/垂直分割与嵌套
# setSizes / sizes 程序化控制各区域宽度
# setCollapsible 禁止折叠特定区域
# saveState / restoreState 持久化分割比例

cmake_minimum_required(VERSION 3.26)

project(42-qsplitter-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
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
