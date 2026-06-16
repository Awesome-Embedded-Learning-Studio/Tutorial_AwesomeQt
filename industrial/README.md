# AwesomeQt Industrial

工业级整机应用模板（HMI / SCADA / automotive 等）。整机允许 QtCharts，复用 widget 栏自绘控件。扁平目录结构 `industrial/<整机名>/`。

## 目录结构

```
industrial/
├── CMakeLists.txt          # 根配置（C++17 / AUTOMOC / find_package Qt6）
└── hmi-dashboard/          # 单个整机（扁平，无类目前缀）
    ├── CMakeLists.txt      # add_subdirectory(demo)
    └── demo/               # 演示可执行程序
```

整机清单见 [todo/registries/05-industrial.md](../todo/registries/05-industrial.md)（hmi-dashboard / scada-panel / automotive-cluster / smart-home-controller / medical-monitor / pos-terminal）。扁平结构：`industrial/<整机名>/`。

## 跨栏复用契约（重要）

industrial 整机**成品期**将复用 widget 栏的 `speed-meter` / `circle-progress` / `line-chart`（STATIC 库，`AwesomeQt::` 命名空间）。

- **骨架期**：以上 widget 尚未产出，仪表 / 趋势区用**自绘占位**，接口位置（PlaceholderGauge / 趋势 QLabel）已留好，成品期一键替换。
- **复用方式（成品期定）**：industrial 顶层 CMake 用 `add_subdirectory(${CMAKE_SOURCE_DIR}/../widget/speed-meter ...)` 拉兄弟栏 STATIC 库；或远期建仓库根级聚合 CMake 统一收口（四栏 `add_subdirectory`，跨栏 link 用 alias target）。骨架期不引入跨栏依赖，保证构建门零依赖通过。
- 整机允许 `find_package(Qt6 COMPONENTS Charts)`（widget 单控件纯自绘，整机可用 Charts）——成品期按需加，骨架期不引。

## 整机清单

| 整机 | 说明 |
|------|------|
| [hmi-dashboard](hmi-dashboard/) | 温度/压力/流量仪表 + 趋势曲线 + 报警列表（骨架：QSplitter 三区 + 占位自绘，复用依赖待 widget 链就位） |

首波 industrial pilot 见 [todo/instance-library.md](../todo/instance-library.md)。

## 构建

```bash
cd industrial
cmake -B build
cmake --build build
```

demo 可执行位于 `build/hmi-dashboard/demo/hmi_dashboard_demo`。

## 编码规范

- C++17，仅 Qt 6 API；`#pragma once`；`override` 必加
- 信号槽用函数指针语法（非 `SIGNAL`/`SLOT` 宏）；range-based for（非 `Q_FOREACH`）
- 命名：目录/整机 kebab-case、类名 PascalCase、头源文件名与类名对应、成员变量尾下划线
- 整机 demo 工程（不建独立 STATIC 库）；整机允许 QtCharts
