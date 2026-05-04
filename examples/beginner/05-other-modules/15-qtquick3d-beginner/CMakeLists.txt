# QtQuick3D QML 3D 场景基础示例 - CMake 配置文件
# 本示例演示 QtQuick3D 模块的核心功能：
# View3D 创建 3D 视口、Model 加载基础几何体、
# PrincipledMaterial PBR 材质、PerspectiveCamera + DirectionalLight、
# Qt Quick 2D 元素与 3D 场景混合布局

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(qtquick3d_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

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
# REQUIRED 表示必须找到，找不到就报错
find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D)
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
    URI Quick3DScene
    VERSION 1.0
    QML_FILES
        Main.qml
)

# 链接 Qt 库到目标
# PRIVATE 表示链接仅对本目标可见，不传递给依赖者
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Quick      # 链接 Qt6::Quick 模块（QML 引擎）
        Qt6::Quick3D    # 链接 Qt6::Quick3D 模块（3D 渲染）
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
