# Observer Pattern 演示程序：温度 Subject + 多 Observer 面板 + 纯 C++ 对照
qt_add_executable(observer-pattern_demo
    main.cpp
    observer-pattern_window.cpp
    observer-pattern_window.h
)

target_link_libraries(observer-pattern_demo PRIVATE
    awesomeqt_observer-pattern
)
