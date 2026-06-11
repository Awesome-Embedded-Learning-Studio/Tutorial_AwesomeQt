cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(10-qtbluetooth-advanced LANGUAGES CXX)                   # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                             # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                             # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                             # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth)              # 引入 Core 和 Bluetooth 模块

qt_add_executable(${PROJECT_NAME}                                 # 生成可执行文件
    ble_scanner.h                                                 # BLE 扫描器头文件（MOC 扫描用）
    ble_scanner.cpp                                               # BLE 扫描器实现
    gatt_client.h                                                 # GATT 客户端头文件（MOC 扫描用）
    gatt_client.cpp                                               # GATT 客户端实现
    main.cpp                                                      # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Bluetooth                              # 链接 Core 和 Bluetooth
)

# 编译器警告标志
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic                                  # GCC/Clang 最高级别常用警告
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4                                                       # MSVC 最高级别常用警告
    )
endif()
