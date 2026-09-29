cmake_minimum_required(VERSION 3.26)                                # Qt6 minimum CMake version
project(15-qtquick3d-advanced LANGUAGES CXX)                         # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                           # C++17 as per project standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                  # Fail if compiler does not support C++17

set(CMAKE_AUTOMOC ON)                                                # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                                # Auto-handle .qrc resource files

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D Qml)     # All modules used by this example

qt_policy(SET QTP0001 NEW)                                           # Use ':/qt/qml/' resource prefix

# Compiler warning flags — consistent across GCC, Clang, MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    add_compile_options(/W4)
endif()

qt_add_executable(${PROJECT_NAME}
    main.cpp                                                         # Application entry point
    scene_config.h                                                   # C++ scene config header (MOC scan)
    scene_config.cpp                                                 # C++ scene config implementation
)

# qt6_add_qml_module registers QML files and C++ types with QML_ELEMENT
# OUTPUT_DIRECTORY controls where generated qmldir and qmlc files go
qt6_add_qml_module(${PROJECT_NAME}
    URI Quick3DAdvanced                                              # QML import URI
    VERSION 1.0                                                      # Module version
    QML_FILES
        Main.qml                                                     # Main 3D scene QML file
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/Quick3DAdvanced                  # Generated files location
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                                    # Core non-GUI functionality
        Qt6::Gui                                                      # Window and image support
        Qt6::Quick                                                    # Qt Quick 2D scene graph
        Qt6::Quick3D                                                  # Qt Quick 3D renderer
        Qt6::Qml                                                      # QML engine and bindings
)
