cmake_minimum_required(VERSION 3.26)                            # Qt6 要求的最低 CMake 版本
project(01-qobject-property-system-advanced LANGUAGES CXX)       # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                            # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                            # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                            # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core)                       # 按需添加模块

qt_add_executable(${PROJECT_NAME}                                # 生成可执行文件
    config_object.h                                               # ConfigObject 类声明
    config_object.cpp                                             # ConfigObject 类实现
    reflective_demo.h                                             # 反射演示辅助函数声明
    reflective_demo.cpp                                           # 反射演示辅助函数实现
    main.cpp                                                      # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core                                             # 按需链接
)
