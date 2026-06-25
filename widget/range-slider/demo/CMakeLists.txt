# RangeSlider 演示程序：静态展示 + 拖动交互 + 程序化设置 + 多配色

qt_add_executable(range_slider_demo
    main.cpp
    range_slider_window.cpp
    range_slider_window.h
)

target_link_libraries(range_slider_demo PRIVATE
    range_slider
)
