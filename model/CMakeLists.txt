cmake_minimum_required(VERSION 3.16)

project(AwesomeQtModels LANGUAGES CXX)

# 全局配置：所有 model 共享（root-owns-config）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# model 子目录：按类目组织 model/<NN-类目>/<模式名>/
# 第一个 model = undo-redo-framework（reference，定范式：库式 STATIC + AwesomeQt:: 命名空间）
add_subdirectory(11-command-pattern/undo-redo-framework)
