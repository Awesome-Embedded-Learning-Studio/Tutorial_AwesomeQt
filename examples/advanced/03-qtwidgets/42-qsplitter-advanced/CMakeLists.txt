cmake_minimum_required(VERSION 3.26)                            # Qt6 required CMake version
project(42-qsplitter-advanced LANGUAGES CXX)                     # Custom QSplitter handle appearance

set(CMAKE_CXX_STANDARD 17)                                       # Unified C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                            # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                            # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                            # Auto-handle .ui form files

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)           # Core, Gui, Widgets modules

qt_add_executable(${PROJECT_NAME}                                # Build executable
    custom_splitter.h                                            # Custom QSplitter subclass header
    custom_splitter.cpp                                          # createHandle() override
    custom_handle.h                                              # Custom QSplitterHandle header
    custom_handle.cpp                                            # paintEvent override
    main.cpp                                                     # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets                      # Link required Qt modules
)
