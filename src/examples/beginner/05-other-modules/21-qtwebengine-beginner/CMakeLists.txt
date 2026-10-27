# QtWebEngine 内嵌浏览器示例 - CMake 配置文件
# 本示例演示 QtWebEngine 模块的核心功能：
# QWebEngineView 加载 URL 与本地 HTML、
# QWebEnginePage::runJavaScript() 注入 JS、
# QWebEngineProfile 管理 Cookie / 缓存

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(qtwebengine_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

# 设置 C++ 标准（Qt 6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求，不回退到低版本

# 自动运行 Qt 的 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTOMOC ON)
# 自动运行 RCC（资源编译器），处理 .qrc 资源文件
set(CMAKE_AUTORCC ON)
# 自动运行 UIC（界面编译器），处理 .ui 界面文件
set(CMAKE_AUTOUIC ON)

# 查找 Qt 6 的 WebEngineWidgets 模块
# WebEngineWidgets 提供 QWebEngineView、QWebEnginePage、QWebEngineProfile
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp  # 主程序源文件
    browserwindow.h  # 浏览器窗口头文件
    browserwindow.cpp  # 浏览器窗口实现
)

# 链接 Qt 库到目标
# PRIVATE 表示链接仅对本目标可见，不传递给依赖者
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::WebEngineWidgets  # 链接 Qt6::WebEngineWidgets 模块（Chromium 内嵌浏览器）
)

# 编译选项：启用所有警告
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
