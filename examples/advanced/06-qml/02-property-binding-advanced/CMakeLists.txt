cmake_minimum_required(VERSION 3.26)                         # Qt6 minimum CMake version
project(02-property-binding-advanced LANGUAGES CXX)            # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                    # C++17 as required by project standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)                           # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                         # Auto-run MOC for Q_OBJECT macro processing

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)      # Modules used by this example

qt_add_executable(${PROJECT_NAME}                             # Build the executable target
    main.cpp                                                  # Application entry point
)

# Embed QML files into a QML module for qrc-based loading
qt6_add_qml_module(${PROJECT_NAME}
    URI PropertyBindingAdvanced                               # Unique QML module identifier
    QML_FILES
        main.qml                                              # Main QML UI file
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                             # Core non-GUI functionality
        Qt6::Gui                                              # GUI types and image support
        Qt6::Quick                                            # QML scene graph and items
        Qt6::Qml                                              # QML engine and bindings
)

# Compiler warning flags — strict warnings on all platforms
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic                             # GCC/Clang: full warning suite
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4                                                  # MSVC: high warning level
    )
endif()
