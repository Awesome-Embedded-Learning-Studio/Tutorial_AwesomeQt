cmake_minimum_required(VERSION 3.26)                           # Qt6 required minimum CMake version
project(08-modbus-advanced LANGUAGES CXX)                        # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                       # Unified C++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)                              # Do not fall back to older standard

set(CMAKE_AUTOMOC ON)                                            # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                            # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                            # Auto-handle .ui form files

find_package(Qt6 REQUIRED COMPONENTS Core SerialBus)             # Core + SerialBus for QModbusDataUnit

qt_add_executable(${PROJECT_NAME}                                # Build executable
    register_map.h                                                # Register mapping table header
    register_map.cpp                                              # Register mapping table implementation
    modbus_manager.h                                              # Modbus client manager header
    modbus_manager.cpp                                            # Modbus client manager implementation
    main.cpp                                                      # Program entry point with 5 demos
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::SerialBus                              # Link Core and SerialBus modules
)

# Compiler warning flags: strict warnings on all platforms
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
