# FadeWidget 演示程序：淡入/淡出按钮 + 时长滑块 + 瞬时透明度滑块

qt_add_executable(fade_animation_demo
    main.cpp
    fade_animation_window.cpp
    fade_animation_window.h
)

target_link_libraries(fade_animation_demo PRIVATE
    fade_animation
)
