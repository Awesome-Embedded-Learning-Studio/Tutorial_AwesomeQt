# EditableTable 演示程序：五类型列 + 增删行 + 打印数据 + 只读切换

qt_add_executable(editable_table_demo
    main.cpp
    editable_table_window.cpp
    editable_table_window.h
)

target_link_libraries(editable_table_demo PRIVATE
    editable_table
)
