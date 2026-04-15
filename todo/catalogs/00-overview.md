# AwesomeQt 代码实例规划 — 总览

> 本文档为 AwesomeQt 项目代码实例的完整规划清单。
> 所有实例统一使用 **Qt 6 + CMake** 构建，第三方依赖通过 CMake FetchContent 自动管理。

## 目标

整合社区优秀的开源 Qt 项目与常见桌面 GUI 需求，
打造涵盖 **1000+ 个 QtWidgets 项目** 的代码实例库，作为教程体系的配套实战资源。

## 规模一览

| 清单 | 分类 | 条目数 |
|------|------|--------|
| [01-widget.md](01-widget.md) | 单个自定义控件（22 子类） | 537 |
| [02-app.md](02-app.md) | 完整应用 Demo（10 子类） | 200 |
| [03-model.md](03-model.md) | 控件组合与设计模式（17 子类） | 317 |
| [04-qml.md](04-qml.md) | QML 专项项目（16 子类） | 52 |
| **合计** | | **1,106** |

## 目录结构

```
Tutorial_AwesomeQt/
├── widget/                              ← 自定义控件
│   ├── button/ label/ input/             ← 基础交互控件
│   ├── progress/ meter/ slider/          ← 进度/仪表/滑块
│   ├── chart/ table/ tree/ list/         ← 数据展示控件
│   ├── calendar/ terminal/               ← 日期/终端控件
│   ├── animation/ opengl/                ← 动画/3D 控件
│   ├── datetime/ network/ multimedia/    ← 专用领域控件
│   └── ... (共 22 个子类)
│
├── app/                                 ← 完整应用
│   ├── dev-tools/       开发工具         (30)
│   ├── network-tools/   网络工具         (20)
│   ├── file-tools/      文件工具         (25)
│   ├── system-tools/    系统工具         (25)
│   ├── image-tools/     图像工具         (20)
│   ├── media-tools/     多媒体工具       (15)
│   ├── office-tools/    办公工具         (15)
│   ├── games/           游戏             (15)
│   ├── security-tools/  安全工具         (10)
│   └── database-tools/  数据库工具       (15)
│
├── model/                               ← 控件组合与设计模式
│   ├── window-framework/    窗口框架     (20)
│   ├── navigation-layout/   导航与布局   (25)
│   ├── page-transition/     页面切换     (15)
│   ├── theme-styling/       主题与样式   (20)
│   ├── notification-dialog/ 通知与弹窗   (15)
│   ├── form-pattern/        表单模式     (20)
│   ├── settings-help-about/ 设置/帮助    (15)
│   ├── print-report/        打印与报表   (15)
│   ├── property-editor/     属性编辑器   (15)
│   ├── drag-drop-pattern/   拖拽模式     (15)
│   ├── command-pattern/     命令模式     (15)
│   ├── data-binding/        数据绑定     (15)
│   ├── chart-integration/   图表集成     (20)
│   ├── utility-pattern/     工具类/模式  (30)
│   ├── complete-ui-demo/    完整界面 Demo (20)
│   ├── context-menu/        右键菜单     (10)
│   └── status-feedback/     状态反馈     (15)
│
├── qml/                                 ← QML 专项 (52)
├── tutorial/                            ← 教程文档（镜像上述目录）
└── todo/                                ← 本规划目录
```

## 编码规范

### CMake 模板

```cmake
cmake_minimum_required(VERSION 3.16)
project(name VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(name
    main.cpp
    Name.h
    Name.cpp
)

target_link_libraries(name PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
```

### 第三方依赖管理

使用 CMake FetchContent 自动拉取，降低配置门槛：

```cmake
include(FetchContent)

FetchContent_Declare(
    dependency_name
    GIT_REPOSITORY https://github.com/owner/repo.git
    GIT_TAG        v1.0.0
)

FetchContent_MakeAvailable(dependency_name)
target_link_libraries(${PROJECT_NAME} PRIVATE dependency_name)
```

### 命名规范

| 范围 | 规范 | 示例 |
|------|------|------|
| 目录名 | kebab-case | `circle-progress` |
| 类名 | PascalCase | `CircleProgress` |
| 头/源文件 | 与类名一致 | `CircleProgress.h` |
| 项目名 | 与目录名一致 | `circle-progress` |

### 代码风格

- C++17 标准，仅使用 Qt 6 API
- 头文件使用 `#pragma once`
- `override` 关键字必须添加
- 信号槽使用函数指针语法，不使用 `SIGNAL()` / `SLOT()` 宏
- 使用 range-based for，不使用 `Q_FOREACH`
- 不使用已废弃 API（`QString::null`、`QRegExp` 等）

## 优先级

| 级别 | 含义 | 说明 |
|------|------|------|
| **P0** | 核心必做 | 高频使用、教程基础依赖、通用性最强 |
| **P1** | 重要推荐 | 常见需求、覆盖面广、工程中大概率遇到 |
| **P2** | 锦上添花 | 特殊场景、低频需求、可作为参考扩展 |

## 参考来源

| 标记 | 来源 | 许可证 |
|------|------|--------|
| TTK | [TTKWidgetTools](https://github.com/Greedysky/TTKWidgetTools) — Greedysky Studio | LGPL-3.0 |
| QWD | [QWidgetDemo](https://github.com/feiyangqingyun/QWidgetDemo) — feiyangqingyun | 源码参考 |
| MTC | [MyTestCode](https://github.com/gongjianbo/MyTestCode) — gongjianbo1992 | 源码参考 |
| GH | GitHub 社区开源项目 | 各项目自有许可证 |
| NEW | 基于常见 GUI 需求全新设计 | — |

> 本项目代码实例为**原创重写**，使用现代 Qt 6 API 与 CMake 构建，与原始参考项目在实现上完全独立。
> 完整致谢请参阅项目 [README.md](../README.md#致谢)。

## 如何参与

- 发现清单中缺失的常见控件/应用/模式？欢迎提 Issue 补充
- 想认领某个 P0/P1 条目进行实现？请先提 Issue 说明意图，避免重复劳动
- 已有实现想要贡献？请遵循上述编码规范，提交 Pull Request
