---
title: "CheckboxTree 手搓手册"
description: "从空 main 一行行搓出 CheckboxTree：3 步打通组合 QTreeWidget 骨架、itemChanged 父子联动、双闸门防信号雪崩。"
---

# CheckboxTree 手搓手册

> **source**：成品答案在 `widget/checkbox-tree/`（做完对照）· **related**：model/view 控件递进链

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这棵勾选树，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **组合而非继承**：继承 QWidget 内含 QTreeWidget 成员，构造 new + parent=this 对象树托管，让 view 自己画
- **QTreeWidgetItem 三态勾选**：setCheckState / checkState，Checked / Unchecked / PartiallyChecked
- **itemChanged 驱动联动**：用户点击 → 槽函数向下传播状态 + 向上重算三态
- **blockSignals 防信号雪崩**：程序化 setCheckState 会再次触发 itemChanged，递归改子孙必须挡信号，否则栈溢出
- **Q_PROPERTY 暴露开关**：propagationEnabled 一个属性在「联动树」和「普通勾选树」之间切换

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
| 1 | 组合 QTreeWidget + addItem 挂节点（先不联动） | [01](./01-skeleton-and-additem.md) |
| 2 | itemChanged 槽 + 向下传播 + 向上回算（核心） | [02](./02-propagation-and-recalc.md) |
| 3 | 双闸门防雪崩 + Q_PROPERTY 联动开关收尾 | [03](./03-signal-guard-and-polish.md) |

成品对照：`widget/checkbox-tree/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **改用 QTreeView + QStandardItemModel**：QTreeWidget 是 Item View 的简化封装，模型大了想共享数据就得换 QTreeView + 自定义 model。思考：勾选联动逻辑从「改 item」挪到「改 model 的 CheckStateRole」，itemChanged 换成 dataChanged 信号，传播/回算算法本体不变。先读 [Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。
- **惰性联动**：现在每次勾选都全量向上回算到根。子树很深时可以只在「子树根」记脏标记，真正读取时再回算——拿复杂度换响应。先想清楚：这种优化在「勾选立刻要看到父态变化」的场景下值不值。
- **持久化勾选集**：把 `checkedItems()` 的结果序列化（按 text 路径而非指针，因为重启后指针失效），下次启动还原。提示：还原时要按路径重新定位 item，再走 `setItemChecked` 触发联动。
