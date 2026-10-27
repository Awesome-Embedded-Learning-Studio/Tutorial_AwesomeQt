# IpEdit 演示程序：输入 + 回显 + 预设 + 清空 + 校验

qt_add_executable(ip_edit_demo
    main.cpp
    ip_edit_window.cpp
    ip_edit_window.h
)

target_link_libraries(ip_edit_demo PRIVATE
    ip_edit
)
