cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(04-qtmultimedia-player-advanced LANGUAGES CXX)            # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                            # 自动运行 MOC，处理 Q_OBJECT

find_package(Qt6 REQUIRED COMPONENTS Core Multimedia)            # 引入 Core 和 Multimedia 模块

qt_add_executable(${PROJECT_NAME}                                # 生成可执行文件
    metadata_reader.h                                            # 元数据读取器头文件
    metadata_reader.cpp                                          # 元数据读取器实现
    playlist_manager.h                                           # 播放列表管理器头文件
    playlist_manager.cpp                                         # 播放列表管理器实现
    main.cpp                                                     # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Multimedia                            # 按需链接 Qt 模块
)

# 编译器警告选项：GCC/Clang 使用 -Wall -Wextra -Wpedantic，MSVC 使用 /W4
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
