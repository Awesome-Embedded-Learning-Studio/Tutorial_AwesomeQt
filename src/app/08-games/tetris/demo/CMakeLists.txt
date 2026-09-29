# Tetris 演示程序：纯 Widgets 自绘棋盘 + 副面板 + 键盘 + 最高分持久化
qt_add_executable(tetris_demo
    main.cpp
    tetris_board.cpp
    tetris_board.h
    tetris_window.cpp
    tetris_window.h
)

target_link_libraries(tetris_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Windows 不弹控制台 / macOS 打包成 .app
set_target_properties(tetris_demo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
