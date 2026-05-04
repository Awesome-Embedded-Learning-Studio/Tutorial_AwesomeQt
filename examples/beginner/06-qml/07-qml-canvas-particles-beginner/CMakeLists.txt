# Qt 6 QML Canvas & Particles 示例 CMake 配置
cmake_minimum_required(VERSION 3.26)
project(QmlCanvasParticlesDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickParticlesPrivate)
qt_policy(SET QTP0001 NEW)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI QmlCanvasParticlesDemo
    VERSION 1.0
    QML_FILES Main.qml
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml/QmlCanvasParticlesDemo
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::QuickParticlesPrivate
)
