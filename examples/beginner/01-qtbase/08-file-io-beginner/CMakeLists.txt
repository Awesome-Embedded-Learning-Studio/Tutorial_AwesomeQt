cmake_minimum_required(VERSION 3.26)                    # Qt6 要求的最低 CMake 版本
project(08-file-io-beginner LANGUAGES CXX)                 # 项目名与语言

set(CMAKE_CXX_STANDARD 23)                                # 使用 C++23 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)                       # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                     # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                     # 自动处理 .qrc 资源文件（本例未使用）
set(CMAKE_AUTOUIC ON)                                     # 自动处理 .ui 文件（本例未使用）

# 查找 Qt6 核心模块
find_package(Qt6 REQUIRED COMPONENTS Core)

# 生成可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp                                              # 示例主程序
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                     # 只需要 QtCore 模块
)

# Windows 下：编译后不显示控制台窗口（如需控制台，注释掉下面这行）
# if(WIN32)
#     set_target_properties(${PROJECT_NAME} PROPERTIES
#         WIN32_EXECUTABLE TRUE
#     )
# endif()
