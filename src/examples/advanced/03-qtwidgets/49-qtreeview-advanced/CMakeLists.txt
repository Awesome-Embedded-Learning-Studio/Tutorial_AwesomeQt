cmake_minimum_required(VERSION 3.26)                    # Qt6 要求的最低 CMake 版本
project(49-qtreeview-advanced LANGUAGES CXX)             # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                               # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                      # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                    # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                    # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                    # 自动处理 .ui 文件

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)   # 本示例需要 Widgets 模块

qt_add_executable(${PROJECT_NAME}                        # 生成可执行文件
    tree_model.h                                         # 树形模型声明（MOC 扫描用）
    tree_model.cpp                                       # 树形模型实现
    widget.h                                             # 主窗口与自定义委托声明
    widget.cpp                                           # 主窗口与自定义委托实现
    main.cpp                                             # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets              # 链接 Qt 模块
)
