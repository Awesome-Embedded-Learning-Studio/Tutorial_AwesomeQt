# CMake 最低版本要求：Qt6 需要 3.16+
cmake_minimum_required(VERSION 3.26)

# 项目声明：12-plugin-beginner，演示插件系统
project(12-plugin-beginner LANGUAGES CXX)

# 设置 C++ 标准：C++17（Qt6 最低要求）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 的自动化 MOC/RCC/UIC 处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt6 的 Core 模块
find_package(Qt6 REQUIRED COMPONENTS Core)

# 定义插件接口的库（作为头文件库）
add_library(textprocessor_interface INTERFACE)
target_include_directories(textprocessor_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 定义插件动态库
qt_add_plugin(uppercase_plugin
    CLASS_NAME UpperCasePlugin
    uppercase_plugin.cpp
)

target_link_libraries(uppercase_plugin
    PRIVATE textprocessor_interface Qt6::Core
)

# 定义第二个插件（插件集示例）
qt_add_plugin(reverse_plugin
    CLASS_NAME ReversePlugin
    reverse_plugin.cpp
)

target_link_libraries(reverse_plugin
    PRIVATE textprocessor_interface Qt6::Core
)

# 主程序可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE textprocessor_interface Qt6::Core
)

# 设置插件输出目录
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/plugins)
file(MAKE_DIRECTORY ${PLUGIN_OUTPUT_DIR})

set_target_properties(uppercase_plugin reverse_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)

# Windows 平台特定设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 编译选项：启用所有警告
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
