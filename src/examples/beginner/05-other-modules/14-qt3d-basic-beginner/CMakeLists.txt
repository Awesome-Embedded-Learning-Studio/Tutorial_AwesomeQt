# Qt 3D 基础场景搭建示例 - CMake 配置文件
# 本示例演示 Qt 3D 模块的核心功能：
# Qt3DWindow 创建 3D 窗口、Entity+Component 的 ECS 架构、
# 基础几何体（球体/立方体/平面）、相机/光源/Phong 材质配置

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(qt3d_basic_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

# 设置 C++ 标准（Qt 6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求，不回退到低版本

# 自动运行 Qt 的 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTOMOC ON)
# 自动运行 RCC（资源编译器），处理 .qrc 资源文件
set(CMAKE_AUTORCC ON)
# 自动运行 UIC（界面编译器），处理 .ui 界面文件
set(CMAKE_AUTOUIC ON)

# 查找 Qt 6 的 3D 相关模块
# REQUIRED 表示必须找到，找不到就报错
find_package(Qt6 REQUIRED COMPONENTS 3DCore 3DExtras 3DInput Widgets)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp       # 主程序源文件
    sceneutils.h   # 场景辅助函数头文件
    sceneutils.cpp # 场景辅助函数实现
)

# 链接 Qt 库到目标
# PRIVATE 表示链接仅对本目标可见，不传递给依赖者
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::3DCore     # 链接 Qt6::3DCore 模块（ECS 架构基础）
        Qt6::3DExtras   # 链接 Qt6::3DExtras 模块（几何体/材质/窗口）
        Qt6::3DInput    # 链接 Qt6::3DInput 模块（鼠标/键盘交互）
        Qt6::Widgets    # 链接 Qt6::Widgets 模块（GUI 界面）
)

# Windows 下设置为控制台应用（方便看到调试输出）
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 编译选项：启用所有警告
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
