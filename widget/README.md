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
| [status-led](status-led/) | 状态指示灯，4 态 + 闪烁动画 |
| [toggle-switch](toggle-switch/) | 滑动开关，点击/拖动 + 滑块动画 |
| [circle-progress](circle-progress/) | 圆形进度环，value/progress 解耦 |
| [speed-meter](speed-meter/) | 速度仪表盘，动画指针 + 双角度自洽 |
| [range-slider](range-slider/) | 双柄范围滑块，拖拽 + 键盘微调 |
| [line-chart](line-chart/) | 折线图，纯 QPainter 自绘 |
| [editable-table](editable-table/) | 可编辑表格，委托校验 + 数据往返 |
| [checkbox-tree](checkbox-tree/) | 树形复选框，三态 + 父子联动 |
| [checkbox-list](checkbox-list/) | 复选框列表，全选 + 级联 |
| [log-viewer](log-viewer/) | 滚动日志，级别染色 + 裁旧 |
| [password-edit](password-edit/) | 密码框，显隐 + 强度指示 |
| [ip-edit](ip-edit/) | IPv4 输入框，4 段跳焦 + 校验 |
| [fade-animation](fade-animation/) | 淡入淡出容器，OpacityEffect |

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
