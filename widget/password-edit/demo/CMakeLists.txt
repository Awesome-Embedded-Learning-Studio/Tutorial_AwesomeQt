# PasswordEdit 演示程序：显隐切换 + 强度实时变 + 回显 + 勾选联动

qt_add_executable(password_edit_demo
    main.cpp
    password_edit_window.cpp
    password_edit_window.h
)

target_link_libraries(password_edit_demo PRIVATE
    password_edit
)
