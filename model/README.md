# AwesomeQt Models

教学用设计模式 / Model-View 实例集合。每个 model 是一个 STATIC 库 + 独立 demo，统一 `AwesomeQt::` 命名空间，示范可复用的架构模式。

## 目录结构

```
model/
├── CMakeLists.txt                 # 根配置（C++17 / AUTOMOC / find_package Qt6）
└── 11-command-pattern/            # 类目目录（按 model registry 17 类目组织）
    └── undo-redo-framework/       # 单个模式实例（reference）
        ├── CMakeLists.txt         # STATIC 库 + add_subdirectory(demo)
        ├── include/               # 公开头文件
        ├── src/                   # 库实现
        └── demo/                  # 演示可执行程序
```

类目目录按 [todo/registries/03-model.md](../todo/registries/03-model.md) 的 17 类目组织。model 路径固定两层：`model/<NN-类目>/<模式名>/`。

## 范式已定（2026-06-16，undo-redo-framework reference 打穿）

model 栏三个待定项已拍死，后续 model 全照此复刻：

| 项 | 结论 |
|---|---|
| 目录布局 | `model/<NN-类目>/<名>/include \| src \| demo`（与 widget 同构） |
| 命名空间 | 库类进 `AwesomeQt::`，demo window 类放全局（与 widget 一致） |
| demo 形态 | 库式 STATIC + 独立 demo（`qt_add_executable` link 库） |

## 实例清单

| 模式 | 类目 | 说明 |
|------|------|------|
| [undo-redo-framework](11-command-pattern/undo-redo-framework/) | 11-command-pattern | QUndoCommand + QUndoStack + QUndoView，命令模式 reference |

首波 model 见 [todo/instance-library.md](../todo/instance-library.md)，按 MV链 / 设计模式链分批往里填。

## 与专家源码篇的边界

pimpl / shared-data / observer / state-machine 这几个模式对齐专家层源码拆解（见 [expert.md](../todo/expert.md) 03/04/19 篇）。model 栏做**练手实例**，专家层拆**源码行号**，二者不重复——文档须明确边界。

## 构建

```bash
cd model
cmake -B build
cmake --build build
```

demo 可执行位于 `build/11-command-pattern/undo-redo-framework/demo/undo_redo_framework_demo`。

## 编码规范

- C++17，仅 Qt 6 API；`#pragma once`；`override` 必加
- 信号槽用函数指针语法（非 `SIGNAL`/`SLOT` 宏）；range-based for（非 `Q_FOREACH`）
- 命名：目录/模式 kebab-case、类名 PascalCase、头源文件名与类名对应、成员变量尾下划线
- STATIC 库 + demo 子目录，库类进 `AwesomeQt::` 命名空间
