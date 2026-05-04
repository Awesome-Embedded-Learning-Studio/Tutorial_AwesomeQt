# Qt 6 容器入门示例
# 演示 QList、QMap、QHash、QSet 和隐式共享的基本用法

cmake_minimum_required(VERSION 3.26)                    # Qt6 要求的最低 CMake 版本
project(04_container_beginner LANGUAGES CXX)             # 项目名与语言

set(CMAKE_CXX_STANDARD 23)                               # 使用 C++23 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)                      # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                    # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                    # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                    # 自动处理 .ui 文件（如有）

# 查找 Qt6 核心模块
find_package(Qt6 REQUIRED COMPONENTS Core)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp                                             # 示例主程序
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                    # 链接 Qt6::Core
)
