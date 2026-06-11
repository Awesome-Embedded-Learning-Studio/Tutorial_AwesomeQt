cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(19-qtwebsockets-server-advanced LANGUAGES CXX)            # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                             # 自动运行 MOC，处理 Q_OBJECT

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)             # 按需添加模块

qt_add_executable(${PROJECT_NAME}                                 # 生成可执行文件
    chat_server.h                                                 # ChatServer 类声明
    chat_server.cpp                                               # ChatServer 类实现
    main.cpp                                                      # 程序入口，演示场景
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::WebSockets                             # 按需链接
)

# 编译器警告标志：GCC/Clang 开启 -Wall -Wextra -Wpedantic，MSVC 开启 /W4
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
