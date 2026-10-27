# HMI Dashboard 演示程序：QSplitter 三区布局 + 占位自绘仪表（骨架）
qt_add_executable(hmi_dashboard_demo
    main.cpp
    hmi_dashboard_window.cpp
    hmi_dashboard_window.h
)

target_link_libraries(hmi_dashboard_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
