# Qt 内存管理入门示例 CMake 配置
# 最小化 CMake 配置，演示 Qt 内存管理核心概念

cmake_minimum_required(VERSION 3.26)                    # Qt 6.9.1 要求的最低 CMake 版本
project(06-memory-management-beginner LANGUAGES CXX)    # 项目名称和语言

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)                              # 使用 C++23
set(CMAKE_CXX_STANDARD_REQUIRED ON)                     # 强制要求，不回退

# Qt 的自动处理选项
set(CMAKE_AUTOMOC ON)                                   # 自动运行 MOC，处理 Q_OBJECT 宏
set(CMAKE_AUTORCC ON)                                   # 自动处理 .qrc 资源文件（如有）
set(CMAKE_AUTOUIC ON)                                   # 自动处理 .ui 文件（如有）

# 查找 Qt 6 核心模块
find_package(Qt6 REQUIRED COMPONENTS Core)              # Core 模块包含 QSharedPointer 等

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp                                            # 示例源代码
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                   # 链接 Qt6::Core 模块
)

# Windows 下设置为 GUI 应用（不显示控制台窗口）
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()
