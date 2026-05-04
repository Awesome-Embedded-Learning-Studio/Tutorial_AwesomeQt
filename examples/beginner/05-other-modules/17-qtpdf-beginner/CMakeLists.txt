# QtPdf PDF 渲染基础示例 - CMake 配置文件
# 本示例演示 QtPdf 模块的核心功能：
# QPdfDocument 加载 PDF 文件、QPdfPageRenderer 渲染页面为 QImage、
# QPdfView 控件直接显示 PDF、页面导航与缩放控制

cmake_minimum_required(VERSION 3.26)  # Qt 6 要求的最低 CMake 版本

project(qtpdf_beginner VERSION 1.0 LANGUAGES CXX)  # 项目名称和语言

# 设置 C++ 标准（Qt 6 需要 C++17 或更高）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求，不回退到低版本

# 自动运行 Qt 的 MOC（元对象编译器），处理 Q_OBJECT 宏
set(CMAKE_AUTOMOC ON)
# 自动运行 RCC（资源编译器），处理 .qrc 资源文件
set(CMAKE_AUTORCC ON)
# 自动运行 UIC（界面编译器），处理 .ui 界面文件
set(CMAKE_AUTOUIC ON)

# 查找 Qt 6 的 Pdf 和 Widgets 模块
# Pdf 模块包含 QPdfDocument、QPdfPageRenderer、QPdfView
# Widgets 模块提供 QWidget、QToolBar 等 UI 组件
find_package(Qt6 REQUIRED COMPONENTS Pdf PdfWidgets Widgets)

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp        # 主程序源文件
    pdfviewer.h     # PDF 查看器主窗口头文件
    pdfviewer.cpp   # PDF 查看器主窗口实现
)

# 链接 Qt 库到目标
# PRIVATE 表示链接仅对本目标可见，不传递给依赖者
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Pdf       # 链接 Qt6::Pdf 模块（PDF 加载与渲染）
        Qt6::PdfWidgets # 链接 Qt6::PdfWidgets 模块（QPdfView 控件）
        Qt6::Widgets   # 链接 Qt6::Widgets 模块（GUI 控件）
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
