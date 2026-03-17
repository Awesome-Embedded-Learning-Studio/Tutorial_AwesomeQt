# Qt 多线程基础示例 - CMake 配置文件
# 本示例演示 QThread、QThreadPool、QtConcurrent 和 QFuture 的使用

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(multithreading_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

# 设置 C++ 标准（Qt 6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求，不回退到低版本

# 自动运行 Qt 的 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTOMOC ON)
# 自动运行 RCC（资源编译器），处理 .qrc 资源文件
set(CMAKE_AUTORCC ON)
# 自动运行 UIC（界面编译器），处理 .ui 界面文件
set(CMAKE_AUTOUIC ON)

# 查找 Qt 6 的 Core 模块（多线程相关类在 Core 模块中）
# REQUIRED 表示必须找到，找不到就报错
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp  # 主程序源文件
)

# 链接 Qt 库到目标
# PRIVATE 表示链接仅对本目标可见，不传递给依赖者
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core        # 链接 Qt6::Core 模块（包含 QThread、QThreadPool 等）
        Qt6::Concurrent  # 链接 Qt6::Concurrent 模块（包含 QtConcurrent）
)

# Windows 下设置为 GUI 应用（不显示控制台窗口）
# 如果希望看到控制台输出，注释掉下面几行
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 编译选项：启用所有警告
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
