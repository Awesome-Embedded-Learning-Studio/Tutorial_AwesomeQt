# Toast Notification 演示程序：四类按钮触发不同 Toast + 连发看堆叠

qt_add_executable(toast-notification_demo
    main.cpp
    toast-notification_window.cpp
    toast-notification_window.h
)

target_link_libraries(toast-notification_demo PRIVATE
    awesomeqt_toast-notification
)
