# Qt Spatial Audio 空间音频示例 - CMake 配置文件
# 本示例演示 Qt SpatialAudio 模块的核心功能：
# QAudioEngine 音频引擎初始化、
# QSpatialSound 设置 3D 音源位置、
# QAudioListener 听者位置与方向、
# 程序生成 WAV 文件 + 空间化播放

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(qtspatial_audio_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

# 设置 C++ 标准（Qt 6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求，不回退到低版本

# 自动运行 Qt 的 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTOMOC ON)
# 自动运行 RCC（资源编译器），处理 .qrc 资源文件
set(CMAKE_AUTORCC ON)
# 自动运行 UIC（界面编译器），处理 .ui 界面文件
set(CMAKE_AUTOUIC ON)

# 查找 Qt 6 的 SpatialAudio 模块
find_package(Qt6 REQUIRED COMPONENTS SpatialAudio)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp  # 主程序源文件
    wavutils.h  # WAV 工具函数头文件
    wavutils.cpp  # WAV 工具函数实现
)

# 链接 Qt 库到目标
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::SpatialAudio  # 链接 Qt6::SpatialAudio 模块（3D 空间音频）
)

# 编译选项：启用所有警告
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
