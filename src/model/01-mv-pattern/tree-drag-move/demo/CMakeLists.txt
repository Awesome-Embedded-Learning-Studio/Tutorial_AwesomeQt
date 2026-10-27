# TreeDragMove 演示程序：单树内拖动改层级 + 双树之间转移节点
qt_add_executable(tree-drag-move_demo
    main.cpp
    tree-drag-move_window.cpp
    tree-drag-move_window.h
)

target_link_libraries(tree-drag-move_demo PRIVATE
    awesomeqt_tree-drag-move
)
