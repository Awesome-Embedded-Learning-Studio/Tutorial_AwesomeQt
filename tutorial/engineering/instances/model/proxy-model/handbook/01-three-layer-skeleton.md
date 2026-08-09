---
title: "Step 1-2：搭三层骨架 + 证明代理不存数据"
description: "建 SortFilterProxyModel 子类，接成 源→代理→视图 三层；改源模型数据证明代理不存数据、自动刷新。"
---

# Step 1-2：搭三层骨架 + 证明代理不存数据

← [手册首页](./index.md) · 下一步 [Step 3-4 自定义排序](./02-custom-sort.md) →

## Step 1：搭三层骨架——源 → 代理（先空壳）→ 视图

### 目标

屏幕上一个表格，能看到数据，但数据是**经过代理**的（不是直接挂源模型）。代理这一层先什么都不重写，就是一个空壳子类，证明三层能通。

### 提示

- 新建 `SortFilterProxyModel : public QSortFilterProxyModel`，放进 `AwesomeQt::` 命名空间
- 头里加 `Q_OBJECT`（后面要加 `Q_PROPERTY`/`Q_ENUM`，moc 要认）——别等报错才补
- CMake 里：`add_library(awesomeqt_proxy-model STATIC ...)` + `target_include_directories(PUBLIC include)` + `add_library(AwesomeQt::proxy-model ALIAS awesomeqt_proxy-model)`
- demo 里三步接线：`proxy->setSourceModel(source)` 然后 `view->setModel(proxy)`——**视图挂的是 proxy，不是 source**
- 构造几个有姓名/年龄/城市的行（年龄故意混进 `"100"`/`"9"`，下一步排序会用到）

### 检查点

跑起来表格能看到 8 行数据，点表头还能排序（基类自带）= 三层通了，代理空壳挂中间没问题。

> Model/View 三层不熟？先读 [Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)。

### 对照答案

- 类声明 + Q_OBJECT：`include/proxy-model.h:25-26`
- setSourceModel 接源：`demo/proxy-model_window.cpp:61`
- 视图挂 proxy：`demo/proxy-model_window.cpp:101`

---

## Step 2：证明代理不存数据（改源、proxy 自动刷新）

### 目标

做一个动作证明「代理不持有数据副本」：在运行时往**源模型**插一行（或改一格），**不碰代理**，看视图是否自动更新。如果代理有影子副本，就得手动同步；如果代理只是映射，源一动它就跟着动。

### 提示

- 加个按钮，点了 `source_model_->appendRow(...)` 插一行
- 不要给代理写任何「同步」代码——它本来就不需要
- 原理：`QSortFilterProxyModel` 在 `setSourceModel` 时连了源模型的信号（`dataChanged`/`rowsInserted`/...），源一变，代理内部重算映射、发自己的信号，视图刷新

### 检查点

点按钮后新行**立刻**出现在表格里（按当前排序规则插到正确位置），代理一行同步代码都没写 = 证明代理只是映射层，零数据冗余。

> 信号驱动刷新不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)、[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。

### 对照答案

- 三层链的关系图与 setSourceModel 时机：见 [成品导览 §2 架构总览](../#_2-架构总览)
- 代理不存数据的设计决策：见 [成品导览 §3 决策①](../#_3-关键设计决策)

---

下一步是重头戏：[Step 3-4 重写 lessThan 做自定义排序](./02-custom-sort.md)。
