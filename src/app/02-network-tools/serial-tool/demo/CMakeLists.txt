# Serial Tool 演示程序：QMainWindow + 配置面板 + 收发区 + Hex/ASCII
# 额外组件 SerialPort 在本目录 find_package（顶层 app/CMakeLists 只提供 Core/Gui/Widgets）
find_package(Qt6 REQUIRED COMPONENTS SerialPort)

qt_add_executable(serial-tool_demo
    main.cpp
    serial_tool_window.cpp
    serial_tool_window.h
)

target_link_libraries(serial-tool_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
)

# Windows 不弹控制台 / macOS 打包成 .app
set_target_properties(serial-tool_demo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
