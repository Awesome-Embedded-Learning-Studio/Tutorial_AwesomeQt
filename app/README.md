# AwesomeQt Apps

教学用整机应用集合。每个 app 是独立的可运行 demo（QMainWindow 主窗口应用），撑起实例库「真库不是片段」的定位。

## 目录结构

```
app/
├── CMakeLists.txt                 # 根配置（C++17 / AUTOMOC / find_package Qt6）
└── 05-image-tools/                # 类目目录（按 app registry 10 类目组织）
    └── image-viewer/              # 单个应用
        ├── CMakeLists.txt         # add_subdirectory(demo)
        └── demo/                  # 演示可执行程序
```

类目目录按 [todo/registries/02-app.md](../todo/registries/02-app.md) 的 10 类目组织（`01-dev-tools` … `10-database-tools`）。app 路径固定两层：`app/<NN-类目>/<应用名>/`。

根 owns config：C++ 标准、AUTOMOC、`find_package(Qt6)` 都在根设一次，子目录只声明 demo。需要额外 Qt 组件的 app（如 sqlite-browser 要 Sql、network-tool 要 Network）在该 app 自己的目录里按需 `find_package(Qt6 REQUIRED COMPONENTS Sql)` 再 link，不堆进顶层默认集。

## 应用清单

| 应用 | 类目 | 说明 |
|------|------|------|
| [image-viewer](05-image-tools/image-viewer/) | 05-image-tools | 图片查看器（缩放/旋转/翻页/幻灯片） |
| [json-editor](01-dev-tools/json-editor/) | 01-dev-tools | JSON 编辑器（格式化/校验/树形浏览） |
| [sqlite-browser](10-database-tools/sqlite-browser/) | 10-database-tools | SQLite 浏览器（表浏览 + 任意 SQL） |
| [serial-tool](02-network-tools/serial-tool/) | 02-network-tools | 串口调试助手（收发 + Hex/ASCII） |
| [network-tool](02-network-tools/network-tool/) | 02-network-tools | TCP/UDP 调试（Server/Client） |
| [tetris](08-games/tetris/) | 08-games | 俄罗斯方块（自绘 + 消行计分） |
| [cpu-memory-monitor](04-system-tools/cpu-memory-monitor/) | 04-system-tools | CPU/内存监控（进度条 + 历史曲线） |

首波旗舰应用已齐，详见 [todo/instance-library.md](../todo/instance-library.md)。

## 构建

```bash
cd app
cmake -B build
cmake --build build
```

demo 可执行位于 `build/05-image-tools/image-viewer/demo/image_viewer_demo`。

## 编码规范

- C++17，仅 Qt 6 API；`#pragma once`；`override` 必加
- 信号槽用函数指针语法（非 `SIGNAL`/`SLOT` 宏）；range-based for（非 `Q_FOREACH`）
- 命名：目录/应用 kebab-case、类名 PascalCase、头源文件名与类名对应、成员变量尾下划线
- 整机 demo 工程（不建 STATIC 库）
