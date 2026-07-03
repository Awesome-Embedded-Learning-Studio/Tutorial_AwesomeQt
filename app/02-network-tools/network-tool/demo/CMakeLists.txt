# Network Tool 演示程序：QMainWindow + 配置面板 + 收发区 + 协议装配
# 额外组件 Network 在本目录 find_package（顶层 app/CMakeLists 只提供 Core/Gui/Widgets）
find_package(Qt6 REQUIRED COMPONENTS Network)

qt_add_executable(network-tool_demo
    main.cpp
    network_tool_window.cpp
    network_tool_window.h
)

target_link_libraries(network-tool_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# Windows 不弹控制台 / macOS 打包成 .app
set_target_properties(network-tool_demo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
