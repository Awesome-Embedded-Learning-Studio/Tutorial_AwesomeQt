cmake_minimum_required(VERSION 3.16)

project(AwesomeQtWidgets LANGUAGES CXX)

# 全局配置：所有子控件共享（root-owns-config）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 控件子目录：每个控件自洽（STATIC 库 + 独立 demo）
add_subdirectory(status-led)
add_subdirectory(toggle-switch)
# add_subdirectory(circle-progress)
