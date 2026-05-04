# Qt SerialPort 基础示例 - CMake 配置文件
# 本示例演示 QSerialPort 和 QSerialPortInfo 的串口枚举、配置与异步收发

cmake_minimum_required(VERSION 3.26)

project(serial_port_beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core SerialPort)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    serialportlist.h
    serialportlist.cpp
    serialportconfigdemo.h
    serialportconfigdemo.cpp
    serialmanager.h
    serialmanager.cpp
    frameparsingdemo.h
    frameparsingdemo.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::SerialPort
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
