cmake_minimum_required(VERSION 3.26)                    # Qt6 要求的最低 CMake 版本
project(12-plugin-advanced LANGUAGES CXX)                # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                               # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                      # 强制要求，不回退
set(CMAKE_CXX_EXTENSIONS OFF)                             # 使用标准 C++，不用编译器扩展

set(CMAKE_AUTOMOC ON)                                    # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                    # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                    # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core)               # 查找 Qt6 Core 模块

qt_add_executable(${PROJECT_NAME}                        # 生成可执行文件
    main.cpp                                             # 程序入口
    plugin_interface.h                                   # 插件接口声明
    plugin_manager.h                                     # 插件管理器声明
    plugin_manager.cpp                                   # 插件管理器实现
    sample_plugin.h                                      # 示例插件声明
    sample_plugin.cpp                                    # 示例插件实现
    sample_plugin.json                                   # 插件元数据
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                    # 链接 Qt6 Core
)

# Windows 平台特定设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE                           # 控制台应用，不弹窗口
    )
endif()

# 编译选项：启用所有警告
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)  # MSVC 警告等级 4
else()
    target_compile_options(${PROJECT_NAME} PRIVATE       # GCC/Clang 警告选项
        -Wall -Wextra -Wpedantic)
endif()
