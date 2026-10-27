# QtWidgets 入门示例 15: QAbstractItemView 视图基类
# setModel() / setSelectionModel() 绑定模型与选择模型
# 选择模式：SingleSelection / MultiSelection / ExtendedSelection
# currentIndex() / selectedIndexes() 获取选中项
# setItemDelegate() 设置自定义委托

cmake_minimum_required(VERSION 3.26)

project(15-qabstractitemview-base-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    date_delegate.h
    date_delegate.cpp
    demo_window.h
    demo_window.cpp
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
