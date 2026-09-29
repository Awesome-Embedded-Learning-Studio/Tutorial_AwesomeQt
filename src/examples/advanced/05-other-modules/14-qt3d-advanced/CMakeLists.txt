cmake_minimum_required(VERSION 3.26)                           # Qt6 要求的最低 CMake 版本
project(14-qt3d-advanced LANGUAGES CXX)                          # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                                       # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                            # 自动运行 MOC，处理 Q_OBJECT

find_package(Qt6 REQUIRED COMPONENTS Core Gui 3DCore 3DExtras)   # 引入 Qt3D 所需模块

qt_add_executable(${PROJECT_NAME}                                # 生成可执行文件
    scene_builder.h                                               # SceneBuilder 类声明
    scene_builder.cpp                                             # SceneBuilder 类实现
    main.cpp                                                     # 程序入口
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                                # 核心非 GUI 功能
        Qt6::Gui                                                 # GUI 基础类型
        Qt6::3DCore                                              # Qt3D 核心实体与组件
        Qt6::3DExtras                                            # Qt3D 预置几何体、材质、窗口
)

# 编译器警告：GCC/Clang 开启 -Wall -Wextra -Wpedantic，MSVC 开启 /W4
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
