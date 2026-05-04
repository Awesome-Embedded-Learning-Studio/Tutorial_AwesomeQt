# QtWidgets 入门示例 41: QStackedWidget 堆叠页面控件
# addWidget 添加页面 / setCurrentIndex 切换页面
# 与 QComboBox / QListWidget 组合做导航菜单
# currentChanged 信号响应页面切换
# 区别于 QTabWidget（无标签头，适合自定义导航）

cmake_minimum_required(VERSION 3.26)

project(41-qstackedwidget-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    StackedWidgetDemoWidget.cpp
    StackedWidgetDemoWidget.h
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
