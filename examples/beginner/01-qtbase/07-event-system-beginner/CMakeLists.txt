# Qt 事件系统入门示例 - CMake 配置
# 最低 CMake 版本要求（Qt6 需要）
cmake_minimum_required(VERSION 3.26)

# 项目名称和语言
project(event-system-beginner LANGUAGES CXX)

# C++23 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt MOC/RCC/UIC 自动处理
# MOC: 处理 Q_OBJECT 宏，生成元对象代码
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt6 模块：Core 核心模块，GUI 模块（QKeyEvent 等事件类）
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

# 生成可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp  # 示例源代码
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core  # 核心模块，包含事件循环和 QEvent 基类
    PRIVATE Qt6::Gui   # GUI 模块，包含 QKeyEvent 等具体事件类
)

# Windows 下设置为 GUI 应用（无控制台窗口）
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 安装规则（可选）
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
