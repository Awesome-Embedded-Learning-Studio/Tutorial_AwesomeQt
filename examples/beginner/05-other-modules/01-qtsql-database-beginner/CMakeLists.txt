cmake_minimum_required(VERSION 3.26)
project(01-qtsql-database-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# QtSql 模块：包含 QSqlDatabase、QSqlQuery、QSqlError 等数据库操作类
find_package(Qt6 REQUIRED COMPONENTS Core Sql)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    Demo.h
    Demo.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Sql
)

# Windows 下保持控制台可见
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
