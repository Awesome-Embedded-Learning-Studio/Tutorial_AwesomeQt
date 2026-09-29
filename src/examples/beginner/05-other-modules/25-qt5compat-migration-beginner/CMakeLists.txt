# Qt5Compat 迁移对照示例 - CMake 配置文件
# 本示例演示 Core5Compat 模块的迁移对照：
# QRegExp → QRegularExpression 迁移、
# QTextCodec → QStringConverter 迁移、
# 渐进式迁移策略的实际操作

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(qt5compat_migration_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

# 设置 C++ 标准（Qt 6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求，不回退到低版本

# 自动运行 Qt 的 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTOMOC ON)
# 自动运行 RCC（资源编译器），处理 .qrc 资源文件
set(CMAKE_AUTORCC ON)
# 自动运行 UIC（界面编译器），处理 .ui 界面文件
set(CMAKE_AUTOUIC ON)

# 查找 Qt 6 的 Core 和 Core5Compat 模块
# Core5Compat 提供 Qt 5 旧 API 兼容（QRegExp、QTextCodec 等）
find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp  # 主程序源文件
    migrationutils.h  # 迁移对照工具函数头文件
    migrationutils.cpp  # 迁移对照工具函数实现
)

# 链接 Qt 库到目标
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core          # Qt 核心模块
        Qt6::Core5Compat   # Qt 5 兼容层（迁移完成后可移除）
)

# 编译选项：启用所有警告
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
