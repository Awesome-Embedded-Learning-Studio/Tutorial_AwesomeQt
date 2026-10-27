# Qt 6 QML 语法基础示例 CMake 配置
cmake_minimum_required(VERSION 3.26)
project(QmlSyntaxDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_policy(SET QTP0001 NEW)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI QmlSyntaxDemo
    VERSION 1.0
    QML_FILES Main.qml
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml/QmlSyntaxDemo
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick
)
