# Proxy-Model 演示程序：源 QStandardItemModel + AwesomeQt::SortFilterProxyModel 代理 + QTableView
qt_add_executable(proxy-model_demo
    main.cpp
    proxy-model_window.cpp
    proxy-model_window.h
)

target_link_libraries(proxy-model_demo PRIVATE
    awesomeqt_proxy-model
)
