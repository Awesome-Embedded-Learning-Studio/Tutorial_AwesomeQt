# CMake 最低版本要求：Qt6 需要 3.16+，这里使用 3.26
cmake_minimum_required(VERSION 3.26)

# 项目声明：02-signal-slot-beginner，使用 C++ 语言
project(02-signal-slot-beginner LANGUAGES CXX)

# 设置 C++ 标准：C++23（Qt6 推荐）
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # 强制要求，不允许降级
set(CMAKE_CXX_EXTENSIONS OFF)          # 使用标准 C++，不用编译器扩展

# Qt 的自动化 MOC/RCC/UIC 处理
set(CMAKE_AUTOMOC ON)      # 自动运行 MOC（处理 Q_OBJECT 宏）
set(CMAKE_AUTORCC ON)      # 自动处理资源文件（.qrc）
set(CMAKE_AUTOUIC ON)      # 自动处理 UI 文件（.ui）

# 查找 Qt6 的 Core 模块（信号槽只需要 Core）
find_package(Qt6 REQUIRED COMPONENTS Core)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp
    counter.h
    slotreceiver.h
    worker.h
    lambdademo.h
)

# 链接 Qt6::Core 库
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core
)

# Windows 平台特定设置：将 Qt 的 DLL 复制到可执行文件目录
if(WIN32)
    # 确保可以找到 Qt 的 DLL
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
