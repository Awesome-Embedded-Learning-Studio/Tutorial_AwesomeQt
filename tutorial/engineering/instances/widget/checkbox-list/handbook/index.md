---
title: "CheckboxList 手搓手册"
description: "从空 main 一行行搓出 CheckboxList：3 步打通组合 QListWidget 骨架、勾选 API 与 itemChanged 转发、批量方法 blockSignals 防信号雪崩。"
---

# CheckboxList 手搓手册

> **source**：成品答案在 `widget/checkbox-list/`（做完对照）· **related**：model/view 控件递进链（进阶版 [checkbox-tree](../) 做父子联动）

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这张勾选清单，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **组合而非继承**：继承 QWidget 内含 QListWidget 成员，构造 new + parent=this 对象树托管，让 view 自己画
- **QListWidgetItem 两态勾选**：`setFlags(ItemIsUserCheckable)` 装复选框、`setCheckState` 设初值、`checkState` 读态
- **itemChanged 转发**：用户点击 → 槽函数转发为更易用的 `checkedChanged(item, bool)`
- **blockSignals 防信号雪崩**：批量 `setCheckState` 每项都回灌 `itemChanged`，整段挡信号才能安静地批量改
- **Q_PROPERTY 暴露开关**：alternatingRowColors / spacing 两个属性可被 Designer 或外部驱动

本件是 [checkbox-tree](../) 的扁平简化版——那件要加父子三态联动和双闸门防递归栈溢出，本件没有层级，一道 `blockSignals` 就够，是入门批量守卫的最佳练手件。

## 1. 起点

先有个能跑的空壳。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    w.resize(100, 100);
    w.show();
    return app.exec();
}
```

弹出空白窗 = 环境通了，往下走。Qt 环境不熟先看 [QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)、[对象树与所有权](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)。

## 2. 任务清单

分 3 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 组合 QListWidget + addItem 装复选框（先不接信号） | [01](./01-skeleton-and-additem.md) |
| 2 | itemChanged 槽 + 状态汇总 checkedTexts/checkedItems | [02](./02-itemchanged-and-collect.md) |
| 3 | 批量方法 blockSignals 守卫 + Q_PROPERTY 收尾 | [03](./03-batch-guard-and-polish.md) |

成品对照：`widget/checkbox-list/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **升级成父子联动树**：把本件改成带父子三态联动，就是 [checkbox-tree](../) 的形态。思考：扁平列表加一层 parent/child 关系后，`itemChanged` 槽要补 `propagateDown`（向下传播）和 `recalcUp`（向上回算）两套递归，批量守卫也要升级成 `is_propagating_` 标志位 + `blockSignals` 的双闸门——递归改子项时不挡就会栈溢出。先读 [Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。
- **换 QListView + QStandardItemModel**：QListWidget 是 Item View 的简化封装，项数大了想共享数据就得换 QTreeView+自定义 model（其实扁平就是 QListView）。思考：勾选逻辑从「改 item」挪到「改 model 的 CheckStateRole」，`itemChanged` 换成 `dataChanged` 信号，批量守卫照样用 `blockSignals`。先读 [Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。
- **持久化勾选集**：把 `checkedTexts()` 序列化（按文本而非指针，重启后指针失效），下次启动还原。提示：还原时按文本重新定位 item，再走 `setItemChecked`——注意它故意不守卫会发 `checkedChanged`，还原期要不要临时挡信号避免刷屏要想清楚。
