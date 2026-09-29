# LineChart 演示程序：静态数据 + 追加随机点 + 重置 + 外观开关

qt_add_executable(line_chart_demo
    main.cpp
    line_chart_window.cpp
    line_chart_window.h
)

target_link_libraries(line_chart_demo PRIVATE
    line_chart
)
