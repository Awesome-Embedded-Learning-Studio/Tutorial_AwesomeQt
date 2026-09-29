# SQLite Browser 演示程序：QMainWindow + 菜单/工具栏/状态栏 + 表列表 + 可编辑表格 + 任意 SQL
# 额外组件 Sql 在本目录 find_package（顶层 app/CMakeLists 只提供 Core/Gui/Widgets）
find_package(Qt6 REQUIRED COMPONENTS Sql)

qt_add_executable(sqlite-browser_demo
    main.cpp
    sqlite_browser_window.cpp
    sqlite_browser_window.h
)

target_link_libraries(sqlite-browser_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

# Windows 不弹控制台 / macOS 打包成 .app
set_target_properties(sqlite-browser_demo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
