cmake_minimum_required(VERSION 3.26)                             # Qt6 minimum CMake version
project(06-qml-model-view-advanced LANGUAGES CXX)                 # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                        # Unified C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # No fallback to older standards

set(CMAKE_AUTOMOC ON)                                             # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                             # Auto-compile .qrc resources
set(CMAKE_AUTOUIC ON)                                             # Auto-process .ui files (if any)

# Compiler warning flags — strict on GCC/Clang, /W4 on MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml QmlIntegration)  # Modules used by this example

qt_add_executable(${PROJECT_NAME}                                 # Build the executable
    contact_model.h                                                # Contact model header (MOC scan)
    contact_model.cpp                                              # Contact model implementation
    sort_filter_model.h                                            # Proxy model header (MOC scan)
    sort_filter_model.cpp                                          # Proxy model implementation
    main.cpp                                                       # Application entry point
)

# Embed QML files into the binary as a QML module
qt6_add_qml_module(${PROJECT_NAME}
    URI QmlModelViewAdvanced                                       # QML import URI
    VERSION 1.0
    QML_FILES main.qml                                             # Main QML UI file
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                                  # Core non-GUI classes
        Qt6::Gui                                                   # GUI types (QColor, QFont, etc.)
        Qt6::Quick                                                 # QML scene graph & basic items
        Qt6::Qml                                                   # QML engine & type registration
        Qt6::QmlIntegration                                        # QML_ELEMENT / QML_VALUE_TYPE macros
)
