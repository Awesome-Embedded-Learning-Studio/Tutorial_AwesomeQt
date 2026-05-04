# AwesomeQt 入门示例 - 根 CMakeLists
# 用于统一构建所有 beginner 示例项目
#
# 用法:
#   cmake -B build -S examples/beginner
#   cmake --build build

cmake_minimum_required(VERSION 3.26)
project(AwesomeQt-Beginner-Examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 统一输出目录：所有可执行文件输出到 build/bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(01-qtbase)
add_subdirectory(02-qtgui)
add_subdirectory(03-qtwidgets)
add_subdirectory(04-qtnetwork)
add_subdirectory(05-other-modules)
add_subdirectory(06-qml)
