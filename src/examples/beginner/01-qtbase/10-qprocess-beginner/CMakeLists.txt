# QProcess 入门示例 - CMake 构建配置
# Qt6 要求的最低 CMake 版本
cmake_minimum_required(VERSION 3.26)

# 项目名称与语言
project(10-qprocess-beginner LANGUAGES CXX)

# 设置 C++ 标准（Qt6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动运行 MOC（处理 Q_OBJECT 宏）
set(CMAKE_AUTOMOC ON)

# 自动运行 RCC（处理 .qrc 资源文件，本示例未使用）
set(CMAKE_AUTORCC ON)

# 自动运行 UIC（处理 .ui 文件，本示例未使用）
set(CMAKE_AUTOUIC ON)

# 查找 Qt6 的 Core 模块（QProcess 在 Core 模块中）
find_package(Qt6 REQUIRED COMPONENTS Core)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp    # 主程序文件
)

# 链接 Qt 库（只需要 Core 模块）
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core
)

# Windows 下设置为 GUI 应用（不显示控制台窗口）
# 如果需要看到控制台输出，注释掉下面这行
if(WIN32)
    # set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
endif()
