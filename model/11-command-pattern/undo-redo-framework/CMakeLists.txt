# Undo/Redo Framework——STATIC 库 + 独立 demo（model 栏 reference 范式）
# 全局配置（C++ 标准 / AUTOMOC / find_package）由根 model/CMakeLists.txt 提供

add_library(move_command STATIC
    include/move_command.h
    src/move_command.cpp
)

target_include_directories(move_command PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(move_command PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_subdirectory(demo)
