# Toast Notification——STATIC 库 + 独立 demo（model 栏设计模式实例）
# 自包含：cmake -S model/12-design-patterns/toast-notification 能单独构建；
# 亦可被根 model/CMakeLists.txt 当 add_subdirectory 复用（guard 避免重复 project）。
if(NOT PROJECT_NAME)
    cmake_minimum_required(VERSION 3.16)

    project(awesomeqt_toast-notification LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
endif()

add_library(awesomeqt_toast-notification STATIC
    include/toast-notification.h
    src/toast-notification.cpp
)

add_library(AwesomeQt::toast-notification ALIAS awesomeqt_toast-notification)

target_include_directories(awesomeqt_toast-notification PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(awesomeqt_toast-notification PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
