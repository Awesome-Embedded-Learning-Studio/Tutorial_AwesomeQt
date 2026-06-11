cmake_minimum_required(VERSION 3.26)                             # Qt6 required minimum
project(22-qtremoteobjects-advanced LANGUAGES CXX)                # Project name matches directory

set(CMAKE_CXX_STANDARD 17)                                        # Unified C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # Do not fall back

set(CMAKE_AUTOMOC ON)                                             # Auto-run MOC for Q_OBJECT
set(CMAKE_AUTORCC ON)                                             # Auto-handle .qrc resources
set(CMAKE_AUTOUIC ON)                                             # Auto-handle .ui files

find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)          # Core + RemoteObjects modules

qt_add_executable(${PROJECT_NAME}                                 # Build executable
    sensor_source.h                                                # Source class declaration
    sensor_source.cpp                                              # Source class implementation
    replica_monitor.h                                              # Replica monitor declaration
    replica_monitor.cpp                                            # Replica monitor implementation
    main.cpp                                                       # Application entry point
)

# Generate source-side and replica-side headers from the .rep API file.
# repc produces rep_sensor_source_source.h and rep_sensor_source_replica.h
# in the build directory, which are included by our hand-written headers.
qt_add_repc_sources(${PROJECT_NAME} sensor_source.rep)            # Source-side generated code
qt_add_repc_replicas(${PROJECT_NAME} sensor_source.rep)           # Replica-side generated code

# The repc-generated headers land in the build directory. Both the
# compiler and AUTOMOC need to find them via an include path.
target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}                             # repc generated headers
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::RemoteObjects                           # Link required Qt modules
)

# Compiler warning flags — strict on all platforms
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
