# ToggleSwitch 演示程序：静态展示 + 交互切换 + 自定义配色

qt_add_executable(toggle_switch_demo
    main.cpp
    toggle_switch_window.cpp
    toggle_switch_window.h
)

target_link_libraries(toggle_switch_demo PRIVATE
    toggle_switch
)
