cmake_minimum_required(VERSION 3.26)                         # Qt6 要求的最低 CMake 版本
project(09-qthread-advanced LANGUAGES CXX)                    # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                    # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                           # 强制要求，不回退
set(CMAKE_CXX_EXTENSIONS OFF)                                 # 使用标准 C++，不用编译器扩展

set(CMAKE_AUTOMOC ON)                                         # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                         # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                         # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)         # 引入 Core 和 Concurrent 模块

qt_add_executable(${PROJECT_NAME}                             # 生成可执行文件
    concurrent_demo.h                                         # QtConcurrent 演示声明
    concurrent_demo.cpp                                       # QtConcurrent 演示实现
    rwlock_demo.h                                             # 读写锁演示声明
    rwlock_demo.cpp                                           # 读写锁演示实现
    threadpool_tasks.h                                        # 线程池任务声明
    threadpool_tasks.cpp                                      # 线程池任务实现
    main.cpp                                                  # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                         # Qt 核心模块
    PRIVATE Qt6::Concurrent                                   # Qt 并发模块
)

if(WIN32)                                                     # Windows 平台特定设置
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

if(MSVC)                                                      # MSVC 启用所有警告
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()                                                        # GCC/Clang 启用所有警告
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
