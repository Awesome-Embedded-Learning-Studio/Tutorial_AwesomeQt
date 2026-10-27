cmake_minimum_required(VERSION 3.26)                    # Qt6 要求的最低 CMake 版本
project(09-mqtt-advanced LANGUAGES CXX)                  # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                               # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                      # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                    # 自动运行 MOC，处理 Q_OBJECT

find_package(Qt6 REQUIRED COMPONENTS Core Mqtt)          # 引入 Core 和 Mqtt 模块

qt_add_executable(${PROJECT_NAME}                        # 生成可执行文件
    mqtt_client_demo.h                                   # MQTT 客户端封装类声明
    mqtt_client_demo.cpp                                 # MQTT 客户端封装类实现
    main.cpp                                             # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Mqtt                          # 链接 Core 和 Mqtt 库
)

# GCC / Clang 编译警告
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic                         # 开启全部常用警告
    )
endif()

# MSVC 编译警告
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4                                              # MSVC 最高实用警告等级
    )
endif()
