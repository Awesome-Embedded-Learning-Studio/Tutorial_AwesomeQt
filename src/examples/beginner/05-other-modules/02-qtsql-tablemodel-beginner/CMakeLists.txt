cmake_minimum_required(VERSION 3.26)
project(02-qtsql-tablemodel-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# QtSql: QSqlTableModel / QSqlDatabase / QSqlQuery
# QtWidgets: QTableView / QMainWindow / 工具栏等 GUI 组件
find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    Database.h
    Database.cpp
    MainWindow.h
    MainWindow.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Sql
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
