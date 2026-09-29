# CheckboxList 演示程序：勾选交互 + 状态汇总 + 批量操作

qt_add_executable(checkbox_list_demo
    main.cpp
    checkbox_list_window.cpp
    checkbox_list_window.h
)

target_link_libraries(checkbox_list_demo PRIVATE
    checkbox_list
)
