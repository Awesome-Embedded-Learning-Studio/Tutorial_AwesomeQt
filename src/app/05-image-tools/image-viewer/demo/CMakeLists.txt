# Image Viewer 演示程序：QMainWindow + 菜单/工具栏/状态栏 + 画布 + 翻页 + 幻灯片
qt_add_executable(image_viewer_demo
    main.cpp
    image_viewer_window.cpp
    image_viewer_window.h
    image_view.cpp
    image_view.h
)

target_link_libraries(image_viewer_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Windows 不弹控制台 / macOS 打包成 .app
set_target_properties(image_viewer_demo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
