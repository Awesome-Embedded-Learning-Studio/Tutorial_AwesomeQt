# QtWidgets 入门示例 24: QPlainTextEdit 纯文本高性能编辑器
# appendPlainText 追加日志的正确用法
# setMaximumBlockCount 限制行数防内存溢出
# highlightCurrentLine 实现行高亮效果

cmake_minimum_required(VERSION 3.26)

project(24-qplaintextedit-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    highlight_plain_text_edit.h
    highlight_plain_text_edit.cpp
    log_terminal.h
    log_terminal.cpp
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
