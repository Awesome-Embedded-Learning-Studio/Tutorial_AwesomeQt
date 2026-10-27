# Observer Pattern——STATIC 库 + 独立 demo（model 栏设计模式实例）
# 自包含：cmake -S model/12-design-patterns/observer-pattern 能单独构建；
# 亦可被根 model/CMakeLists.txt 当 add_subdirectory 复用（guard 避免重复 project）。
if(NOT PROJECT_NAME)
    cmake_minimum_required(VERSION 3.16)

    project(awesomeqt_observer-pattern LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
endif()

add_library(awesomeqt_observer-pattern STATIC
    include/observer-pattern.h
    src/observer-pattern.cpp
)

add_library(AwesomeQt::observer-pattern ALIAS awesomeqt_observer-pattern)

target_include_directories(awesomeqt_observer-pattern PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(awesomeqt_observer-pattern PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
