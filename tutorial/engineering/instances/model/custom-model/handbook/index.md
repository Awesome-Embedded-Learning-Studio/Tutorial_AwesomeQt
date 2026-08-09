---
title: "CustomTableModel 手搓手册"
description: "从空 main 一行行搓出 CustomTableModel：3 阶段打通自定义表格模型骨架、多角色 data、可编辑 setData、增删行 begin/end 信号。"
---

# CustomTableModel 手搓手册

> **source**：成品答案在 `model/01-mv-pattern/custom-model/`（做完对照）· **related**：Model/View 章第 1 环

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 CustomTableModel，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **自定义模型骨架**：继承 `QAbstractTableModel` + 重写 `rowCount`/`columnCount`/`data` 三件套
- **多角色 data**：一个单元格同时回答 DisplayRole/EditRole/TextAlignmentRole/BackgroundRole/ToolTipRole 等多种问题
- **可编辑模型**：`flags` 给 `ItemIsEditable` + `setData` 回写数据 + 发 `dataChanged`
- **增删行协议**：`insertRows`/`removeRows` 用 `beginInsertRows`/`endInsertRows` 包夹容器改动
- **表头**：`headerData` 给列标题和行号
- **模型与视图解耦**：QTableView 只通过模型接口拿数，换数据源不动视图

## 1. 起点

先有个能跑的空壳。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QTableView>
#include <QWidget>
#include <QVBoxLayout>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    auto* layout = new QVBoxLayout(&w);
    layout->addWidget(new QTableView); // 先放个空表视图
    w.resize(500, 300);
    w.show();
    return app.exec();
}
```

弹出空白表 = 环境通了，往下走。Model/View 概念不熟先看 [Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md) 和 [QTableView 基础](../../../../../beginner/03-qtwidgets/51-qtableview-beginner.md)。

## 2. 任务清单

分 3 阶段 6 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| 阶段 | Step | 目标 | 进 |
|---|---|---|---|
| 阶段一：骨架 | 1 | 定义 Task 结构体 + 列枚举 | [01](./01-skeleton-and-roles.md) |
| | 2 | rowCount/columnCount/data(DisplayRole) 让表有内容 | [01](./01-skeleton-and-roles.md) |
| 阶段二：能力 | 3 | 多角色 data（对齐/底色/悬停） + headerData | [02](./02-edit-and-roles.md) |
| | 4 | 可编辑：flags + setData + dataChanged | [02](./02-edit-and-roles.md) |
| 阶段三：动态 | 5 | insertRows/removeRows 配 begin/end 信号 | [03](./03-insert-remove-and-polish.md) |
| | 6 | 便捷 API appendTask + demo 增删行按钮 | [03](./03-insert-remove-and-polish.md) |

成品对照：`model/01-mv-pattern/custom-model/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **排序**：重写 `sort(int column, Qt::SortOrder)`，按列对 `tasks_` 重排，发 `layoutAboutToBeChanged`/`layoutChanged`。思考：排序要不要保留原行号映射？
- **过滤**：再子类化 `QSortFilterProxyModel`，按「只看未完成」过滤。提示：代理模型不改源数据，是插在视图和源模型之间的一层。
- **布尔列用复选框**：step 4 的 done 列改成 `Qt::CheckStateRole` 显示勾选框，setData 同时响应 EditRole 和 CheckStateRole。这是另一种编辑交互范式。
- **下一站**：代理模型（proxy model）/ 自定义委托（QStyledItemDelegate）——换数据层换编辑器，模型骨架不变。
