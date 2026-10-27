# CustomTableModel——QAbstractTableModel 子类化自管数据（model 栏 reference 范式）
# 自包含配置：cmake -S model/01-mv-pattern/custom-model -B build 可单独构建
cmake_minimum_required(VERSION 3.16)

project(awesomeqt_custom-model LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_library(awesomeqt_custom-model STATIC
    include/custom-model.h
    src/custom-model.cpp
)

add_library(AwesomeQt::custom-model ALIAS awesomeqt_custom-model)

target_include_directories(awesomeqt_custom-model PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(awesomeqt_custom-model PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# 进阶教学用：开启更严格的诊断，便于读者第一时间发现 API 误用
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(awesomeqt_custom-model PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_subdirectory(demo)
