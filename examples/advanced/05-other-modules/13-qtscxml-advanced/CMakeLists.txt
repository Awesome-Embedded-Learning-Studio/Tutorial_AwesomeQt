cmake_minimum_required(VERSION 3.26)                               # Qt6 minimum CMake version
project(13-qtscxml-advanced LANGUAGES CXX)                          # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                          # Use C++17 as required
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                 # Do not fall back to older standards
set(CMAKE_AUTOMOC ON)                                               # Auto-run MOC for Q_OBJECT classes

# ---- Compiler warning flags (GCC/Clang and MSVC) ----
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    add_compile_options(/W4)
endif()

# ---- Find Qt6 modules ----
find_package(Qt6 REQUIRED COMPONENTS Core Scxml)                    # Core for QCoreApplication, Scxml for SCXML

# ---- Define source directory for SCXML file fallback lookup ----
# main.cpp uses SCXML_SOURCE_DIR to find the .scxml file at runtime
set(SCXML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# ---- Configure header with the source path ----
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/scxml_source_path.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/scxml_source_path.h"
)

# ---- Executable ----
qt_add_executable(${PROJECT_NAME}
    main.cpp                                                         # Application entry point
    microwave_controller.h                                           # Controller class declaration
    microwave_controller.cpp                                         # Controller class implementation
    microwave.scxml                                                  # SCXML state machine definition
)

target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")                           # For generated scxml_source_path.h

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                                    # QCoreApplication, QTimer, etc.
        Qt6::Scxml                                                   # QScxmlStateMachine, data model
)

# ---- Copy SCXML file to build output directory ----
# Ensures the .scxml file is alongside the executable at runtime
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/microwave.scxml"
    "${CMAKE_CURRENT_BINARY_DIR}/microwave.scxml"
    COPYONLY
)
