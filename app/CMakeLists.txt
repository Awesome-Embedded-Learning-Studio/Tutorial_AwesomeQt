cmake_minimum_required(VERSION 3.16)

project(AwesomeQtApps LANGUAGES CXX)

# 全局配置：所有 app 共享（root-owns-config）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# app 子目录：按类目组织 app/<NN-类目>/<应用名>/，每个 app 自洽（整机 demo）
# 需要额外 Qt 组件的 app（如 sqlite-browser 要 Sql、network-tool 要 Network）
# 在该 app 自己的目录里按需 find_package + link，不要堆进顶层默认集
add_subdirectory(05-image-tools/image-viewer)
