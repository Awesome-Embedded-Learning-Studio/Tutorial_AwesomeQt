# TreeDragMove——STATIC 库 + 独立 demo（model/01-mv-pattern/拖拽移动节点）
# 自包含配置：cmake -S model/01-mv-pattern/tree-drag-move -B build 可单独构建
cmake_minimum_required(VERSION 3.16)

project(tree_drag_move LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 库目标：kebab 目录名 → 下划线 target 名（CMake target 不含连字符）
add_library(awesomeqt_tree-drag-move STATIC
    include/tree-drag-move.h
    src/tree-drag-move.cpp
)

target_include_directories(awesomeqt_tree-drag-move PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(awesomeqt_tree-drag-move PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# AwesomeQt:: 命名空间别名（实例库统一对外接口）
add_library(AwesomeQt::tree-drag-move ALIAS awesomeqt_tree-drag-move)

add_subdirectory(demo)
