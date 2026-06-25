# LogViewer 演示程序：级别按钮 + 连发压测 + 清空 + autoScroll 切换

qt_add_executable(log_viewer_demo
    main.cpp
    log_viewer_window.cpp
    log_viewer_window.h
)

target_link_libraries(log_viewer_demo PRIVATE
    log_viewer
)
