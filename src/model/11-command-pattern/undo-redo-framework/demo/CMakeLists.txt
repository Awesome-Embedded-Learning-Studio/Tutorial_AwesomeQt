# Undo/Redo Framework 演示程序：按钮触发 MoveCommand + QUndoStack + QUndoView 可视化
qt_add_executable(undo_redo_framework_demo
    main.cpp
    undo_redo_window.cpp
    undo_redo_window.h
)

target_link_libraries(undo_redo_framework_demo PRIVATE
    move_command
)
