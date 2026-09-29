# CustomTableModel 演示程序：QTableView + 增/删行 + 优先级 SpinBox 代理 + 完成态勾选代理
qt_add_executable(custom-model_demo
    main.cpp
    custom-model_window.cpp
    custom-model_window.h
)

target_link_libraries(custom-model_demo PRIVATE
    awesomeqt_custom-model
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(custom-model_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()
