# Qt 6 字符串与编码示例 CMake 配置
# 展示 QString、QByteArray、QStringView 的使用

cmake_minimum_required(VERSION 3.26)                    # Qt 6.9.1 要求的最低 CMake 版本
project(StringEncodingDemo VERSION 1.0 LANGUAGES CXX)   # 项目名称与语言

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)                              # 使用 C++23
set(CMAKE_CXX_STANDARD_REQUIRED ON)                     # 强制要求，不回退

# 自动处理 Qt 的 MOC、RCC、UIC
set(CMAKE_AUTOMOC ON)                                   # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                   # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                   # 自动处理 .ui 文件（如有）

# 查找 Qt6 核心模块
find_package(Qt6 REQUIRED COMPONENTS Core)              # 只需要 Core 模块

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp                                            # 示例源代码
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                   # 链接 Qt6::Core
)

# Windows 下设置为控制台应用（可选）
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()
