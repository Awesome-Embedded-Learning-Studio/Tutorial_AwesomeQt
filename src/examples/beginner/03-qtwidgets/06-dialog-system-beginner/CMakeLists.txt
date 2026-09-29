# QtWidgets 入门示例 06: 对话框体系基础
# QDialog exec() vs show() / QDialogButtonBox / 自定义对话框布局与数据返回 / accept() reject()
# 个人信息编辑对话框

cmake_minimum_required(VERSION 3.26)

project(06-dialog-system-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    userinfo.h
    userinfodialog.h
    userinfodialog.cpp
    finddialog.h
    finddialog.cpp
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
