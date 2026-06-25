# EditableTable 控件——STATIC 库 + 独立 demo
# 全局配置（C++ 标准 / AUTOMOC / find_package）由根 widget/CMakeLists.txt 提供

add_library(editable_table STATIC
    include/editable_table.h
    src/editable_table.cpp
)

target_include_directories(editable_table PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(editable_table PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
