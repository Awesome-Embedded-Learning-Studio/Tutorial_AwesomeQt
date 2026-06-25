# CheckboxTree 控件——STATIC 库 + 独立 demo
# 全局配置（C++ 标准 / AUTOMOC / find_package）由根 widget/CMakeLists.txt 提供

add_library(checkbox_tree STATIC
    include/checkbox_tree.h
    src/checkbox_tree.cpp
)

target_include_directories(checkbox_tree PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(checkbox_tree PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
