# AwesomeQt Widgets

教学用自定义控件集合。每个控件是独立的 STATIC 库 + 可运行 demo，统一使用 `AwesomeQt::` 命名空间。

## 目录结构

```
widget/
├── CMakeLists.txt          # 根配置（C++17 / AUTOMOC / find_package Qt6）
└── status-led/             # 单个控件
    ├── CMakeLists.txt      # STATIC 库 + add_subdirectory(demo)
    ├── include/            # 公开头文件
    ├── src/                # 库实现
    └── demo/               # 演示可执行程序
```

根 owns config：C++ 标准、AUTOMOC、`find_package(Qt6)` 都在根 `CMakeLists.txt` 设一次，子目录只声明库与 demo。

## 控件清单

| 控件 | 说明 |
|------|------|
| [status-led](status-led/) | 状态指示灯，4 种状态（Normal/Warning/Error/Offline）+ 闪烁动画 |
| [toggle-switch](toggle-switch/) | 滑动开关，点击/拖动切换 + 滑块滑动动画 + 轨道变色 + 自定义配色 |

## 构建

```bash
cd widget
cmake -B build
cmake --build build
```

demo 可执行位于 `build/status-led/demo/status_led_demo`。

## 编码规范

- C++17，仅 Qt 6 API；`#pragma once`；`override` 必加
- 信号槽用函数指针语法（非 `SIGNAL`/`SLOT` 宏）；range-based for（非 `Q_FOREACH`）
- 命名：目录/控件 kebab-case、类名 PascalCase、头源文件名与类名对应、成员变量尾下划线
- STATIC 库 + demo 子目录，不维护 SHARED/install/MERGE_MODE
