cmake_minimum_required(VERSION 3.26)                              # Qt6 minimum CMake
project(05-qtmultimedia-camera-advanced LANGUAGES CXX)             # Project name = directory name

set(CMAKE_CXX_STANDARD 17)                                        # C++17 as per coding rules
set(CMAKE_CXX_STANDARD_REQUIRED ON)                               # No fallback to older standards

set(CMAKE_AUTOMOC ON)                                             # Auto-run MOC for Q_OBJECT macros
set(CMAKE_AUTORCC ON)                                             # Auto-handle .qrc resource files
set(CMAKE_AUTOUIC ON)                                             # Auto-handle .ui designer files

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia)         # Core + pixel types + camera APIs

qt_add_executable(${PROJECT_NAME}                                 # Qt-aware executable target
    camera_info.h                                                  #   CameraInfo class header
    camera_info.cpp                                                #   CameraInfo class implementation
    frame_processor.h                                              #   FrameProcessor class header
    frame_processor.cpp                                            #   FrameProcessor class implementation
    main.cpp                                                       #   Application entry point
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core                                                  # Event loop, QObject, qDebug
        Qt6::Gui                                                   # QImage, QRgb, pixel types
        Qt6::Multimedia                                           # QCameraDevice, QMediaDevices
)

# Compiler warning flags — strict warnings across all platforms
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
