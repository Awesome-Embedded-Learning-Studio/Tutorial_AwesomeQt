# CMakeLists.txt for Qt6 i18n Beginner Example
# 演示 Qt 国际化(i18n)基础用法：tr()函数、QTranslator、lupdate/lrelease工作流

cmake_minimum_required(VERSION 3.26)                    # Qt6 要求的最低 CMake 版本
project(13-i18n-beginner LANGUAGES CXX)                 # 项目名与语言

set(CMAKE_CXX_STANDARD 23)                              # 使用 C++23 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)                     # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                   # 自动运行 MOC，处理 Q_OBJECT 宏
set(CMAKE_AUTORCC ON)                                   # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                   # 自动处理 .ui 文件（如有）

# 查找 Qt6 核心和 Widgets 模块
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
)

# 创建可执行文件
qt_add_executable(i18n-example
    main.cpp                                # 示例主程序
)

# 链接 Qt 库
target_link_libraries(i18n-example
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

# 注意：完整国际化工作流需要手动执行以下命令：
# 1. lupdate i18n-example.pro -ts translations/i18n-example_zh_CN.ts
#    (从源代码提取需要翻译的字符串)
# 2. 使用 Qt Linguist 编辑 .ts 文件
# 3. lrelease translations/i18n-example_zh_CN.ts
#    (将 .ts 文件编译成 .qm 二进制文件)
#
# 本示例专注于演示 tr() 和 QTranslator 的基本用法，
# 翻译文件的生成过程请参考 Qt 官方文档。

# 设置输出目录
set_target_properties(i18n-example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
