# QtQuick3D Physics 物理模拟基础示例 - CMake 配置文件
# 本示例演示 QtQuick3D Physics 模块的核心功能：
# PhysicsWorld 物理世界初始化、StaticRigidBody/DynamicRigidBody 刚体、
# BoxShape/SphereShape/CapsuleShape 碰撞体、
# 重力/弹性/摩擦力参数配置与物理模拟交互

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(qtquick3d_physics_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

# 设置 C++ 标准（Qt 6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求，不回退到低版本

# 自动运行 Qt 的 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTOMOC ON)
# 自动运行 RCC（资源编译器），处理 .qrc 资源文件
set(CMAKE_AUTORCC ON)
# 自动运行 UIC（界面编译器），处理 .ui 界面文件
set(CMAKE_AUTOUIC ON)

# 查找 Qt 6 的 Quick3D 和 Quick 模块
# QtQuick3D.Physics 是独立模块，需要显式查找
find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D)
find_package(Qt6 OPTIONAL_COMPONENTS Quick3DPhysics)

if(NOT Qt6Quick3DPhysics_FOUND)
    message(WARNING "Qt6Quick3DPhysics not found - skipping qtquick3d-physics example. "
                     "Install QtQuick3DPhysics module to enable this example.")
    return()
endif()

qt_policy(SET QTP0001 NEW)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp  # 主程序源文件
)

# 注册 QML 模块
# URI: QML import 路径
# VERSION: 模块版本号
# QML_FILES: 包含的 QML 文件列表
qt_add_qml_module(${PROJECT_NAME}
    URI PhysicsScene
    VERSION 1.0
    QML_FILES
        Main.qml
)

# 链接 Qt 库到目标
# PRIVATE 表示链接仅对本目标可见，不传递给依赖者
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Quick              # 链接 Qt6::Quick 模块（QML 引擎）
        Qt6::Quick3D            # 链接 Qt6::Quick3D 模块（3D 渲染）
        Qt6::Quick3DPhysics     # 链接 Qt6::Quick3DPhysics 模块（物理模拟）
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
