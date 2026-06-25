---
title: "Step 3：整表数据往返 + Q_PROPERTY + suppress 去重"
description: "setData 按列类型夹值/兜底回填、data() 按列类型还原 QVariant；补 editable/gridVisible/alternatingRowColors 三个 Q_PROPERTY；用 suppress_signal_ 屏蔽程序化回灌。"
---

# Step 3：整表数据往返 + Q_PROPERTY + suppress 去重

← [Step 2](./02-delegate-and-validation.md) · [手册首页](./index.md) →

前两步搭好了「能编辑、能校验」的表。这一步把它变成「能整表存取」的录入控件：外部一次塞一批数据进来、一次取一批出去，中间夹值/兜底全自动。再补三个行为开关 Q_PROPERTY，处理好「程序化填值不该发假编辑通知」的去重。搓完这三件，控件就生产可用了。

## Step 3：setData / data() / Q_PROPERTY / suppress_signal_

### 目标

实现四个东西：

1. **`setData(QVector<QVector<QVariant>>)`**：整表回填。清行留列，按列类型逐格转换——kInt 用 `std::clamp` 夹到 `[min,max]`、kDouble 同理、kCombo 不在候选项回库首项、kCheck 还原 `Qt::Checked/Unchecked`、kText 直写。行数据短于列数时补空占位 item
2. **`data()`**：取回整表，按列类型还原 QVariant——kCheck→bool、kInt/kDouble 转换失败给 `QVariant()`、其余→文本。空表/无列返回空向量
3. **Q_PROPERTY 三件**：`editable` / `gridVisible` / `alternatingRowColors`，透传到 QTableWidget。setEditable 切 `setEditTriggers(NoEditTriggers vs DoubleClicked|SelectedClicked|...)`
4. **suppress_signal_ 去重**：`cellChanged` 连到私有 `onCellChanged`，里面查 suppress 标志——setData/addRow/clear 程序化改动置 true 不回灌，只有用户真实编辑才 emit `dataEdited`

### 提示（按顺序）

1. **setData 入口置 suppress**：`suppress_signal_ = true; table_->setRowCount(0);`（清行留列）。没列直接 return。然后 `setRowCount(rows.size())` 逐行逐列转换填入，结尾 `suppress_signal_ = false`
2. **每列转换分支**：kInt `v.toInt(&ok)` 失败取 min，再 `std::clamp(parsed, (int)min, (int)max)`；kDouble 同理（注意 lo/hi 用 `std::min/max(min,max)` 防 min>max）；kCombo `v.toString()` 不在 combo 里就取 `combo.first()`；kCheck `v.typeId()==Bool ? toBool() : toInt()!=0` 还原。kText 直 `v.toString()`
3. **短行补占位**：填完一行后，对 `c = row_data.size()` 到 `col_count` 的列补 `new QTableWidgetItem(QString())`，否则 `table_->item(r,c)` 返回 null
4. **data() 出口兜底**：每格先 `item ? item->text() : QString()`。kCheck 取 `item ? item->checkState() : Unchecked` 还原 bool。kInt/kDouble `toInt(&ok)` 失败 append `QVariant()`（不是 0——区分「非法」和「真 0」）
5. **Q_PROPERTY 透传 + 提前 return 防抖**：setEditable 里 `if (editable_ == editable) return;`（属性去抖），再 `table_->setEditTriggers(...)`、`emit editableChanged`。gridVisible / alternatingRowColors 同理透传 `setShowGrid` / `setAlternatingRowColors`
6. **cellChanged 连线**：`connect(table_, &QTableWidget::cellChanged, this, &EditableTable::onCellChanged)`。onCellChanged 入口：先 `if (suppress_signal_) return;`，再做 row/col 越界 + item null 检查，最后按列类型取值（kCheck 取 checkState，其余取 text）emit `dataEdited(row, col, value)`
7. **薄透传 getter**：补 `currentRow()`（透传 `table_->currentRow()`）和 `resizeColumnsToContents()`（透传同名），给 demo 用——别暴露 table_ 指针

### 关键认知——为什么 suppress 必须有

`setData` / `addRow` / `clear` 都是程序化建项，每一次 `setItem` 都会触发 `cellChanged`。如果你把 cellChanged 直接连到 `dataEdited` 信号，外部刚 `setData` 一批，立刻接到 N×M 条「假编辑」通知，值还是刚填进去的——业务逻辑被无意义信号刷爆。`suppress_signal_` 就是堵这个洞：程序化填值前置位、后清位，`onCellChanged` 入口先查它，只让 suppress 为 false（用户真实双击编辑）的改动走 emit。这是组合控件配合 view 信号的经典去重套路，凡是「程序化改 model 又监听 model 变化」的场景都要这么干。

### 检查点

- `setData` 传 `{"Bob", 120, 1.5, "Yellow", false}`：表格显示分数 100、比率 1.0、颜色 Red（回库首项）= 夹值/兜底对了
- `data()` 拍回来是二维 `QVariant` 向量，kCheck 列是 bool、kInt/kDouble 列是数值 = 出口还原对了
- 勾上 editable 复选框→表可双击编辑，取消→只读 = Q_PROPERTY 透传对了
- `setData` 后**没有**接到一堆 dataEdited 假通知；双击单元格改一个值→收到一条 dataEdited = suppress 去重对了

> 信号槽 / 去重机制不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。Q_PROPERTY？[属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

### 对照答案

- setData 整表回填 + clamp/兜底：`src/editable_table.cpp:261`
- data() 按列类型还原：`src/editable_table.cpp:334`
- 短行补占位 item：`src/editable_table.cpp:327`
- setEditable 切 EditTriggers + 防抖：`src/editable_table.cpp:397`
- cellChanged 连线：`src/editable_table.cpp:174`
- onCellChanged suppress 去重 + 边界 clamp：`src/editable_table.cpp:444`
- 薄透传 currentRow / resizeColumnsToContents：`src/editable_table.cpp:389` / `src/editable_table.cpp:393`

---

搓完了。跑 demo 对照成品：预填的 Bob 超界行被夹值、双击输 9999 被夹回 100、Print Data 拍出来的二维数据按列类型正确还原、只读切换生效 = 你搓的和 repo 一致。

想再深？回 [手册首页](./index.md) 看进阶挑战（勾选改三态 / 委托加自定义校验器 / 换自定义 model 后端 / 下一站带过滤排序的高级表格）。
