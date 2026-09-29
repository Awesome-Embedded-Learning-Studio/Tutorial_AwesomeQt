cmake_minimum_required(VERSION 3.16)

project(AwesomeQtModels LANGUAGES CXX)

# 全局配置：所有 model 共享（root-owns-config）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# model 子目录：按类目组织 model/<NN-类目>/<模式名>/
# reference = undo-redo-framework（库式 STATIC + AwesomeQt:: 命名空间）
add_subdirectory(11-command-pattern/undo-redo-framework)

# MV 链（01-mv-pattern）
add_subdirectory(01-mv-pattern/proxy-model)
add_subdirectory(01-mv-pattern/custom-model)
add_subdirectory(01-mv-pattern/tree-drag-move)

# 设计模式链（12-design-patterns）
add_subdirectory(12-design-patterns/observer-pattern)
add_subdirectory(12-design-patterns/toast-notification)
