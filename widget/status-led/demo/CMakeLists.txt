# StatusLED 演示程序：静态展示 + 交互切换 + 多尺寸

qt_add_executable(status_led_demo
    main.cpp
    status_led_window.cpp
    status_led_window.h
)

target_link_libraries(status_led_demo PRIVATE
    status_led
)
