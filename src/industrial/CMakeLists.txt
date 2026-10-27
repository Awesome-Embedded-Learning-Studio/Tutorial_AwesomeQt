cmake_minimum_required(VERSION 3.16)

project(AwesomeQtIndustrial LANGUAGES CXX)

# 全局配置：所有整机共享（root-owns-config）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 骨架期仅 Core/Gui/Widgets；整机成品期按需补 Charts（见 README「跨栏复用契约」）
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 整机子目录：扁平结构 industrial/<整机名>/（无类目前缀）
add_subdirectory(hmi-dashboard)
