---
title: "阶段一：自定义模型骨架 + DisplayRole"
description: "定义 Task 结构体和 Column 枚举，子类化 QAbstractTableModel 重写 rowCount/columnCount/data(DisplayRole)，让 QTableView 第一次显示出自管数据。"
---

# 阶段一：自定义模型骨架 + DisplayRole

← [手册首页](./index.md) · 下一步 [阶段二 多角色 + 可编辑](./02-edit-and-roles.md) →

这一阶段的目标只有一个：让 `QTableView` 第一次从「你自己写的模型」里取出数据并显示出来。一旦跑通这步，你就理解了 Model/View 解耦的核心——视图只通过模型接口拿数，模型是数据的唯一真源。

## Step 1：定义 Task 结构体 + 列枚举

### 目标

定义一个普通结构体 `Task`（标题/负责人/优先级/完成态），再给模型类定义一个 `Column` 枚举把列索引固定下来。**这步还没有任何 Qt 代码逻辑**，纯粹是「先想清楚我的数据长什么样、表有几列」。

### 提示

- `Task` 放 `AwesomeQt::` 命名空间里，是普通 POD（4 个成员：`QString title`、`QString assignee`、`int priority`、`bool done`），**不要**继承 QObject
- 模型类继承 `QAbstractTableModel`，加 `Q_OBJECT`（moc 要认）
- 定义 `enum Column { kTitle=0, kAssignee=1, kPriority=2, kDone=3, kColumnCount=4 }`，紧跟 `Q_ENUM(Column)`——列数固定写死在枚举里，改列只动这里
- 加私有成员 `QList<Task> tasks_;`——这就是「自管数据源」
- 构造里填几条种子数据（4 条任务），让跑起来不是空表

### 检查点

编译过（moc 不报错，`Q_OBJECT` 生效）= 骨架立住了。此时视图还是空的（data 还没实现），但模型对象能 new 出来了。

> 信号槽 / 元对象不熟？[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)、[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。

### 对照答案

- Task 结构体四字段：`include/custom-model.h:19`
- Column 枚举 + Q_ENUM：`include/custom-model.h:41-42`
- tasks_ 成员声明：`include/custom-model.h:91`
- 构造填种子数据：`src/custom-model.cpp:18`

---

## Step 2：rowCount / columnCount / data(DisplayRole) 让表有内容

### 目标

重写三个虚函数：`rowCount` 返回行数、`columnCount` 返回列数、`data` 返回单元格文本。跑起来 `QTableView` 第一次显示出那张 4×4 的任务表。

### 提示

- `rowCount(parent)`：**先判断 `parent.isValid()`，有效就返回 0**（表格模型没有父子层级，parent 有效意味着「问子项数」，恒为 0）；否则返回 `tasks_.size()`
- `columnCount(parent)`：同样 parent 有效返回 0，否则返回 `kColumnCount`
- `data(index, role)`：
  - 先做**边界保护**：`index.isValid()` 为假或行列越界，返回空 `QVariant`（`return {}`）
  - 先只处理 `Qt::DisplayRole`（显示文本），其它 role 这步先不管，走 `default: return {}`
  - 按 `index.column()` 用 switch 返回对应字段（title/assignee/priority/done）
  - 布尔列显示「是/否」比 1/0 可读
- demo 端：`model_ = new CustomTableModel(this); table_view_->setModel(model_);`

### 关键认知

- **parent 有效返回 0 不是偷懒**：不这么写，某些视图会把表格当成有子项，rowCount 返回非零 → 视图无限递归铺行（见 [troubleshooting](./troubleshooting.md) 的「rowCount 死循环」）
- **越界 index 必须返回空**：视图在滚动/动画/异步刷新时会塞进脏 index，不保护就是定时炸弹

### 检查点

表里显示 4 行 4 列，标题/负责人/优先级/完成态都正确填了值 = 三大虚函数对了。`setModel` 之后视图就活过来了。

> Model/View 概念不熟？[Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)、[QAbstractItemView 基类](../../../../../beginner/03-qtwidgets/15-qabstractitemview-base-beginner.md)。

### 对照答案

- rowCount（含 parent 判断）：`src/custom-model.cpp:32`
- columnCount：`src/custom-model.cpp:40`
- data 边界保护 + DisplayRole：`src/custom-model.cpp:51,64`
- demo 端 setModel：`demo/custom-model_window.cpp:39`

---

下一步：[阶段二 给 data 加多角色 + 让单元格可编辑](./02-edit-and-roles.md)。
