---
title: "Proxy-Model 手搓手册"
description: "从空 main 一步步搓出 SortFilterProxyModel 子类：三层骨架、自定义 lessThan（数值/不区分大小写）、自定义 filterAcceptsRow（关键词/全列），配 Qt 6.13 过滤通知范式。"
---

# Proxy-Model 手搓手册

> **source**：成品答案在 `model/01-mv-pattern/proxy-model/`（做完对照）· **related**：Model/View 模式（[入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md) · [进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)）

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 Proxy-Model，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **Model/View 三层链**：源模型 → 代理模型 → 视图，代理只做映射不碰数据
- **QSortFilterProxyModel 子类化**：重写 `lessThan`（排序）和 `filterAcceptsRow`（过滤）两个 protected 钩子
- **自定义排序**：数值列按值比（避免字典序陷阱）+ 文本列不区分大小写
- **自定义过滤**：关键词子串匹配，支持单列 / 全列两种范围
- **Qt 6 过滤通知范式**：`beginFilterChange` / `endFilterChange`（取代 Qt 6.13 弃用的 `invalidateFilter`）
- **`AwesomeQt::` 命名空间 + STATIC 库 + 独立 demo 的工程骨架**

## 1. 起点

先有个能跑的空壳——一个 `QTableView` 挂一个原始 `QStandardItemModel`，能看到数据但还不能排序/过滤：

```cpp
#include <QApplication>
#include <QStandardItemModel>
#include <QTableView>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QStandardItemModel model;
    model.setHorizontalHeaderLabels({"Name", "Age", "City"});
    model.appendRow({new QStandardItem("Alice"), new QStandardItem("29"), new QStandardItem("Beijing")});
    QTableView view;
    view.setModel(&model);
    view.show();
    return app.exec();
}
```

看到一行数据 = 环境通了，往下走。Model/View 三层不熟先看 [Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)。

## 2. 任务清单

分 6 步，归到 3 个文件，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 搭三层骨架：源 → 代理（先空壳）→ 视图 | [01](./01-three-layer-skeleton.md) |
| 2 | 证明代理不存数据（改源、proxy 自动刷新） | [01](./01-three-layer-skeleton.md) |
| 3 | 重写 lessThan：数值列按值排 | [02](./02-custom-sort.md) |
| 4 | 文本列不区分大小写 | [02](./02-custom-sort.md) |
| 5 | 重写 filterAcceptsRow：单列 / 全列过滤 | [03](./03-custom-filter.md) |
| 6 | 切换过滤范围用 Qt 6 通知范式（不踩弃用坑） | [03](./03-custom-filter.md) |

成品对照：`model/01-mv-pattern/proxy-model/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **多关键词 AND/OR 过滤**：把搜索框拆成多个词，`filterAcceptsRow` 里按词组合命中逻辑。思考：词与词之间该 AND 还是 OR？怎么暴露给用户？
- **自定义 `QSortFilterProxyModel::Role`**：让某些列按 `Qt::UserRole`（藏的真实排序键）比，而不是 `DisplayRole`（显示文本）。提示：`setSortRole`。
- **接 `QIdentityProxyModel`**：另一种代理——不改行列顺序，只改某列的显示（如给状态列加图标），体会「代理不改顺序」的另一面。
- **下一站**：自定义委托（Delegate，`QStyledItemDelegate`）——代理管「哪行哪列显示什么顺序」，委托管「某格怎么画 / 怎么编辑」，二者正交。
