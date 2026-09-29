# Qt Remote Objects 传感器数据共享示例 - CMake 配置文件
# 本示例演示 Qt RemoteObjects 模块的核心功能：
# REPC 文件定义远程对象接口、
# QRemoteObjectHost 发布 Source 端对象、
# QRemoteObjectNode 获取 Replica 端副本、
# 属性自动同步与远程方法调用

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(qtremoteobjects_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

# 设置 C++ 标准（Qt 6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求，不回退到低版本

# 自动运行 Qt 的 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTOMOC ON)
# 自动运行 RCC（资源编译器），处理 .qrc 资源文件
set(CMAKE_AUTORCC ON)
# 自动运行 UIC（界面编译器），处理 .ui 界面文件
set(CMAKE_AUTOUIC ON)

# 查找 Qt 6 的 RemoteObjects 模块
find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp  # 主程序源文件
    sensorsource.h  # 传感器 Source 端头文件
    sensorsource.cpp  # 传感器 Source 端实现
)

# 确保生成的 repc 头文件可被找到
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# 注册 REPC 文件：同时生成 Source 端和 Replica 端代码
# sensordata.rep 定义了远程对象的属性、信号和槽
qt_add_repc_sources(${PROJECT_NAME} sensordata.rep)
qt_add_repc_replicas(${PROJECT_NAME} sensordata.rep)

# 链接 Qt 库到目标
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core           # Qt 核心模块
        Qt6::RemoteObjects  # Qt 远程对象模块
)

# 编译选项：启用所有警告
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
