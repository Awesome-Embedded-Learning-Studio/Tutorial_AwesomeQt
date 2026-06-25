---
title: "EditableTable 手搓手册"
description: "从空壳 QTableWidget 一行行搓出 EditableTable：3 步打通组合控件骨架、按列声明类型 + 委托校验、整表数据往返 + Q_PROPERTY 行为开关。"
---

# EditableTable 手搓手册

> **source**：成品答案在 `widget/editable-table/`（做完对照）· **related**：model/view 组合控件递进链第 1 环　·　教程层 [QTableWidget 入门](../../../../../beginner/03-qtwidgets/50-qtablewidget-beginner.md) / [Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 EditableTable，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **组合而非继承**：把 QTableWidget 当私有成员挂进 QWidget，不重写 paintEvent——model/view 控件的正确组装姿势
- **自定义委托（QStyledItemDelegate）**：重写 createEditor / setEditorData / setModelData，按列类型分发编辑器并做校验（step 2 重头）
- **按列声明类型 + 委托回调取规格**：用 `std::function` 把列定义喂给委托，避免环引用
- **整表数据往返**：setData 回填按列类型夹值/兜底，data() 取回按列类型还原 QVariant（step 3）
- **Q_PROPERTY 行为开关 + suppress 去重**：editable 等属性透传 view、cellChanged 程序化回灌的屏蔽

## 1. 起点

先有个能跑的空壳：一个 QWidget 里塞个空的 QTableWidget。main 里弹窗：

```cpp
#include <QApplication>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    auto* layout = new QVBoxLayout(&w);
    auto* table = new QTableWidget(&w);
    table->setColumnCount(3);
    table->setRowCount(2);
    layout->addWidget(table);
    w.resize(400, 300);
    w.show();
    return app.exec();
}
```

弹出一张能双击编辑的空表 = 环境通了，往下走。这一步用的是裸 QTableWidget，还没封装——下一步我们就把它包进 EditableTable 类。QTableWidget 不熟先看 [QTableWidget 入门](../../../../../beginner/03-qtwidgets/50-qtablewidget-beginner.md)。

## 2. 任务清单

分 3 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 组合骨架 + 按列声明类型（ColumnType + addColumn + addRow） | [01](./01-compose-and-columns.md) |
| 2 | 委托校验（ValidatorDelegate 按列挑编辑器 + 空值兜底） | [02](./02-delegate-and-validation.md) |
| 3 | 整表数据往返 + Q_PROPERTY 行为开关 + suppress 去重 | [03](./03-data-roundtrip.md) |

成品对照：`widget/editable-table/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **勾选列改成三态**：现在 kCheck 是二值 `Checked/Unchecked`。想支持 `PartiallyChecked`（半选，表示子项部分勾选），要改 `data()` 出口还原逻辑 + addRow 默认态。思考：半选在整表数据往返里怎么表达？用 `int` 而非 `bool` 吗？
- **委托加自定义校验器**：现在数值列靠 QSpinBox 内置夹值。想要更复杂的规则（比如「分数必须 5 的倍数」），可以在 setModelData 里对 `box->value()` 二次校验，不合法则不写入或恢复旧值。提示：用 `model->data(index)` 取旧值做恢复。
- **换成自定义模型后端**：QTableWidget 内部是预设的 model + view 二合一。把它换成 `QTableView` + 自定义 `QAbstractTableModel`，委托这层校验逻辑能原样复用——这是从「便捷控件」走向「真正的 model/view」的关键一跳。参考 [Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。
- **下一站**：带过滤/排序的高级表格控件——复用本件的委托，但在 model 层加 `QSortFilterProxyModel`。
