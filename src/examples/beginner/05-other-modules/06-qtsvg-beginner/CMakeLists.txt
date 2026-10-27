cmake_minimum_required(VERSION 3.26)
project(06-qtsvg-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# QtSvg: QSvgRenderer（核心渲染引擎）
# QtSvgWidgets: QSvgWidget（SVG 显示控件）
# QtWidgets: QApplication / QMainWindow / QWidget 等
find_package(Qt6 REQUIRED COMPONENTS Core Svg SvgWidgets Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    svgviewerwindow.h
    svgviewerwindow.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Svg
        Qt6::SvgWidgets
        Qt6::Widgets
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
