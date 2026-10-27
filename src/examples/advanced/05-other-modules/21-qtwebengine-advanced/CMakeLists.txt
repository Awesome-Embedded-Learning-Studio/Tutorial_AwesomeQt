cmake_minimum_required(VERSION 3.26)                                     # Qt6 minimum CMake version
project(21-qtwebengine-advanced LANGUAGES CXX)                            # Project name and language

set(CMAKE_CXX_STANDARD 17)                                                # Use C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                       # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                                     # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                                     # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                                     # Auto-handle .ui form files

find_package(Qt6 REQUIRED COMPONENTS                                      # Required Qt modules
    Core                                                                  #   Core non-GUI essentials
    Gui                                                                   #   GUI infrastructure
    Widgets                                                               #   QWidget / QMainWindow
    WebEngineWidgets                                                      #   QWebEngineView and profile APIs
)

qt_add_executable(${PROJECT_NAME}                                         # Build the executable
    custom_scheme.h                                                       #   Custom scheme handler header
    custom_scheme.cpp                                                     #   Custom scheme handler implementation
    web_profile_manager.h                                                 #   Profile manager header
    web_profile_manager.cpp                                               #   Profile manager implementation
    main.cpp                                                              #   Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                                         # Core module
        Qt6::Gui                                                          # GUI module
        Qt6::Widgets                                                      # Widgets module
        Qt6::WebEngineWidgets                                             # WebEngineWidgets module
)

# Compiler warning flags — strict warnings for clean code
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
