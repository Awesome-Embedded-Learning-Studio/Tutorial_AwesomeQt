# QtWidgets 入门示例 69: QWizard 向导对话框
# addPage 添加向导页 / initializePage/validatePage 页面生命周期
# registerField/field 页面间数据传递 / 自定义按钮文字与样式

cmake_minimum_required(VERSION 3.26)

project(69-qwizard-beginner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    welcomepage.h
    welcomepage.cpp
    userinfopage.h
    userinfopage.cpp
    preferencepage.h
    preferencepage.cpp
    confirmpage.h
    confirmpage.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets
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
