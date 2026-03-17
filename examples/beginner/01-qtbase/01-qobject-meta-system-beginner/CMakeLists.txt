# QtBase 入门示例 01: QObject 与元对象系统
# 最小化 CMake 配置，可独立构建

# 指定 CMake 最低版本要求（Qt6 要求 3.16+，推荐 3.26）
cmake_minimum_required(VERSION 3.26)

# 项目声明：指定项目名和编程语言
project(01-qobject-meta-system-beginner LANGUAGES CXX)

# 设置 C++ 标准（Qt6 推荐使用 C++17 或更高，这里使用 C++23）
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 强制要求标准，不允许降级
set(CMAKE_CXX_EXTENSIONS OFF)         # 禁用编译器扩展，保证标准一致性

# 关键设置：启用 Qt 的自动化工具
set(CMAKE_AUTOMOC ON)      # 自动运行 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTORCC ON)      # 自动处理资源文件（.qrc），本例不需要但配置备用
set(CMAKE_AUTOUIC ON)      # 自动处理 UI 文件（.ui），本例不需要但配置备用

# 查找 Qt6 的 Core 模块（QObject 是 QtCore 的一部分）
# REQUIRED 表示必须找到，找不到会报错
find_package(Qt6 REQUIRED COMPONENTS Core)

# 创建可执行文件
# main.cpp 是我们的源代码文件
qt_add_executable(${PROJECT_NAME}
    main.cpp
)

# 链接 Qt 库
# PRIVATE 表示这些依赖只在内部使用，不暴露给其他项目
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core
)

# 可选：设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
