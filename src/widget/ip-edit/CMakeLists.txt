# IpEdit 控件——STATIC 库 + 独立 demo
# 全局配置（C++ 标准 / AUTOMOC / find_package）由根 widget/CMakeLists.txt 提供

add_library(ip_edit STATIC
    include/ip_edit.h
    src/ip_edit.cpp
)

target_include_directories(ip_edit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ip_edit PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
