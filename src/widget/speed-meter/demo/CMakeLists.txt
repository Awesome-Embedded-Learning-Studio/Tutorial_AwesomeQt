# SpeedMeter 演示程序：静态几档 + Cycle 过渡 + Slider 驱动 + 随机跳变

qt_add_executable(speed_meter_demo
    main.cpp
    speed_meter_window.cpp
    speed_meter_window.h
)

target_link_libraries(speed_meter_demo PRIVATE
    speed_meter
)
