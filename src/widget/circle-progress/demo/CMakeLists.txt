# CircleProgress 演示程序：静态多档 + Cycle 过渡 + Slider 驱动 + 配色变体

qt_add_executable(circle_progress_demo
    main.cpp
    circle_progress_window.cpp
    circle_progress_window.h
)

target_link_libraries(circle_progress_demo PRIVATE
    circle_progress
)
