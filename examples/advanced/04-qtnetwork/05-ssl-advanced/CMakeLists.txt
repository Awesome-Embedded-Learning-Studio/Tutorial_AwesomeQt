cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(05-ssl-advanced LANGUAGES CXX)                           # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                            # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                            # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                            # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core Network)               # Core + Network 用于 SSL/TLS

# 编译器警告标志：GCC/Clang 启用严格警告
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

qt_add_executable(${PROJECT_NAME}                                # 生成可执行文件
    ssl_analyzer.h                                                # SSL 证书分析器头文件
    ssl_analyzer.cpp                                              # SSL 证书分析器实现
    tls_config_demo.h                                             # TLS 配置演示头文件
    tls_config_demo.cpp                                           # TLS 配置演示实现
    main.cpp                                                      # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Network                                # Qt Core + Network 模块
)
