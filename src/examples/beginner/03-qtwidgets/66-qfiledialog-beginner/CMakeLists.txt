# QtWidgets 入门示例 66: QFileDialog 文件选择对话框
# getOpenFileName / getSaveFileName / getExistingDirectory
# getOpenFileNames 多文件选择 / setNameFilter 文件类型过滤
# setDirectory 默认目录 / QStandardPaths 标准路径

cmake_minimum_required(VERSION 3.26)

project(66-qfiledialog-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
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
