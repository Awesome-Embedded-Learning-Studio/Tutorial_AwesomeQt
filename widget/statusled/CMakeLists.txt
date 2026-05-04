cmake_minimum_required(VERSION 3.16)

project(StatusLED VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Standalone build: include the module directly
if(NOT COMMAND awesomeqt_add_widget)
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/AwesomeQtWidgets.cmake")
endif()

# Build the StatusLED widget library
if(DEFINED AWESOMEQT_USING_MERGE_MODE)
    awesomeqt_add_widget(StatusLED
        MERGE_MODE
        QT_COMPONENTS Core Gui Widgets
    )
else()
    awesomeqt_add_widget(StatusLED
        QT_COMPONENTS Core Gui Widgets
    )
endif()

# Demo executable (individual mode only)
if(NOT DEFINED AWESOMEQT_USING_MERGE_MODE)
    qt_add_executable(statusled_demo main.cpp StatusLEDWindow.cpp)
    target_link_libraries(statusled_demo
        PRIVATE
            StatusLED
    )
    set_target_properties(statusled_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
