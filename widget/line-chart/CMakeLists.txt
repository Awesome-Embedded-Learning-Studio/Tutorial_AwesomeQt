# LineChart 控件——STATIC 库 + 独立 demo
# 全局配置（C++ 标准 / AUTOMOC / find_package）由根 widget/CMakeLists.txt 提供

add_library(line_chart STATIC
    include/line_chart.h
    src/line_chart.cpp
)

target_include_directories(line_chart PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(line_chart PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
