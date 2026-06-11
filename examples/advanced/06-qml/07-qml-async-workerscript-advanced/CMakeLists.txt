cmake_minimum_required(VERSION 3.26)                    # Qt6 要求的最低 CMake 版本
project(07-qml-async-workerscript-advanced LANGUAGES CXX) # 项目名与语言

set(CMAKE_CXX_STANDARD 17)                               # 统一 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                      # 强制要求，不回退

set(CMAKE_AUTOMOC ON)                                    # 自动运行 MOC，处理 Q_OBJECT

# GCC/Clang: 开启严格警告; MSVC: /W4
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    add_compile_options(/W4)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml) # 按需添加模块

qt_add_executable(${PROJECT_NAME}                        # 生成可执行文件
    main.cpp                                             # 程序入口
)

# 将 QML 文件和 worker.js 注册为 QML 模块资源
qt6_add_qml_module(${PROJECT_NAME}
    URI QmlAsyncAdvanced                                 # QML 模块 URI
    VERSION 1.0
    QML_FILES
        main.qml                                         # 主界面
    RESOURCES
        worker.js                                        # WorkerScript 后台脚本
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml       # 按需链接
)
