cmake_minimum_required(VERSION 3.26)
project(51-qtableview-advanced LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)                               # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                      # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                    # 自动运行 MOC，处理 Q_OBJECT
set(CMAKE_AUTORCC ON)                                    # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)                                    # 自动处理 .ui 文件

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)   # 按需添加模块

qt_add_executable(${PROJECT_NAME}                        # 生成可执行文件
    big_table_model.h                                    # 百万行模型声明
    big_table_model.cpp                                  # 百万行模型实现
    main.cpp                                             # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets              # 按需链接
)
