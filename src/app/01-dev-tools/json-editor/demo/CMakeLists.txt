# JSON Editor 演示程序：QMainWindow + 编辑区/树视图 + 校验/格式化/紧凑/打开/保存
qt_add_executable(json-editor_demo
    main.cpp
    json_editor_window.cpp
    json_editor_window.h
)

target_link_libraries(json-editor_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Windows 不弹控制台 / macOS 打包成 .app
set_target_properties(json-editor_demo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
