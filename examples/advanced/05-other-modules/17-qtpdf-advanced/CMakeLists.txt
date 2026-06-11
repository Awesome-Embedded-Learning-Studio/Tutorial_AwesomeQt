cmake_minimum_required(VERSION 3.26)                     # Qt6 requires at least 3.16; 3.26 for robust C++ support
project(17-qtpdf-advanced LANGUAGES CXX)                   # Project name matches directory name

set(CMAKE_CXX_STANDARD 17)                                 # C++17 as required by project conventions
set(CMAKE_CXX_STANDARD_REQUIRED ON)                        # Do not fall back to older standards

set(CMAKE_AUTOMOC ON)                                      # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                      # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                      # Auto-handle .ui form files

# --- Find Qt6 components ----------------------------------------------------
# Pdf provides QPdfDocument for loading and text extraction.
# Gui provides QPdfWriter and QPainter for creating test PDF files.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Pdf)

qt_add_executable(${PROJECT_NAME}                          # Qt-aware executable target
    pdf_search_engine.h                                    # Header — MOC scans for Q_OBJECT
    pdf_search_engine.cpp                                  # PdfSearchEngine implementation
    main.cpp                                               # Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                          # Core non-GUI functionality
        Qt6::Gui                                           # QPdfWriter, QPainter, QPageSize
        Qt6::Pdf                                           # QPdfDocument, QPdfSelection
)

# --- Compiler warning flags --------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
