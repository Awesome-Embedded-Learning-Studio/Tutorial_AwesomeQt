# CheckboxTree 演示程序：三层示例树 + 父子联动观察 + 已勾选项列出 + 全选/全不选 + 联动开关

qt_add_executable(checkbox_tree_demo
    main.cpp
    checkbox_tree_window.cpp
    checkbox_tree_window.h
)

target_link_libraries(checkbox_tree_demo PRIVATE
    checkbox_tree
)
