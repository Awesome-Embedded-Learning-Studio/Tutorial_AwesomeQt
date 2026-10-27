cmake_minimum_required(VERSION 3.26)                        # Qt6 要求的最低 CMake 版本
project(06-drag-drop-advanced LANGUAGES CXX)                 # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                   # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                          # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                        # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                        # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                        # 自动处理 .ui 文件（如有）

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)       # 拖放需要 Gui 和 Widgets

qt_add_executable(${PROJECT_NAME}                            # 生成可执行文件
    drag_list_widget.h                                       # 拖拽源控件头文件
    drag_list_widget.cpp                                     # 拖拽源控件实现
    drop_list_widget.h                                       # 放下目标控件头文件
    drop_list_widget.cpp                                     # 放下目标控件实现
    main.cpp                                                 # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets                  # 链接所需模块
)
