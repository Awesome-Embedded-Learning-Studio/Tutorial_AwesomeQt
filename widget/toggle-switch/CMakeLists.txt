# ToggleSwitch 控件——STATIC 库 + 独立 demo
# 全局配置（C++ 标准 / AUTOMOC / find_package）由根 widget/CMakeLists.txt 提供

add_library(toggle_switch STATIC
    include/toggle_switch.h
    src/toggle_switch.cpp
)

target_include_directories(toggle_switch PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(toggle_switch PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
