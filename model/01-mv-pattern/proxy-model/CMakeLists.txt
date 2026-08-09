# SortFilterProxyModel——STATIC 库 + 独立 demo（model 栏 proxy 范式）
# 本子目录自包含：cmake -S model/01-mv-pattern/proxy-model -B build 可单独构建，
# 不依赖根 model/CMakeLists.txt（全局配置在此自带）。

cmake_minimum_required(VERSION 3.16)

project(AwesomeQtProxyModel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_library(awesomeqt_proxy-model STATIC
    include/proxy-model.h
    src/proxy-model.cpp
)

add_library(AwesomeQt::proxy-model ALIAS awesomeqt_proxy-model)

target_include_directories(awesomeqt_proxy-model PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(awesomeqt_proxy-model PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
