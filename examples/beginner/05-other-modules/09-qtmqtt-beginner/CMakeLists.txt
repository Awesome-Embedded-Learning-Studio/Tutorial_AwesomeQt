cmake_minimum_required(VERSION 3.26)
project(09-qtmqtt-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# QtMqtt: QMqttClient / QMqttSubscription / QMqttTopicFilter 等
# QtWidgets: QApplication / QMainWindow / QWidget 等
find_package(Qt6 REQUIRED COMPONENTS Core Mqtt Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    mqttwindow.h
    mqttwindow.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Mqtt
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
