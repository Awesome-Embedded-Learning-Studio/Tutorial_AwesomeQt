# CPU / Memory Monitor 演示程序：QMainWindow + 进度条 + CPU 历史曲线 + 跨平台系统读取
# 纯 Widgets（Core/Gui/Widgets），无额外 Qt 组件；Windows 分支系统调用走 kernel32。
qt_add_executable(cpu_memory_monitor_demo
    main.cpp
    system_monitor_window.cpp
    system_monitor_window.h
    system_stats.cpp
    system_stats.h
    cpu_history_view.cpp
    cpu_history_view.h
)

target_link_libraries(cpu_memory_monitor_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Windows 分支（GlobalMemoryStatusEx / GetSystemTimes）走 kernel32——仅在 Q_OS_WIN 时需要。
# Linux/WSL 编译时 system_stats.cpp 的整段 Windows 代码被 #ifdef 掉，此链接不触发。
if(WIN32)
    target_link_libraries(cpu_memory_monitor_demo PRIVATE kernel32)
endif()

# Windows 不弹控制台 / macOS 打包成 .app
set_target_properties(cpu_memory_monitor_demo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
