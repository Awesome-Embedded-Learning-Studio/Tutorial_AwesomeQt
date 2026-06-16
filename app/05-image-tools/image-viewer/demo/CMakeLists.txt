# Image Viewer 演示程序：QMainWindow + 菜单 + 文件对话框 + 占位中央区（骨架）
qt_add_executable(image_viewer_demo
    main.cpp
    image_viewer_window.cpp
    image_viewer_window.h
)

target_link_libraries(image_viewer_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
