---
title: "Step 2：委托校验（核心）"
description: "实现 detail::ValidatorDelegate，重写 createEditor / setEditorData / setModelData，按列类型挑编辑器、空值兜底、提交阶段直接写——委托的校验前置套路。"
---

# Step 2：委托校验（核心）

← [Step 1](./01-compose-and-columns.md) · [手册首页](./index.md) · 下一步 [Step 3 整表数据往返](./03-data-roundtrip.md) →

这一步是整个控件的核心——挂上一个自定义委托，让编辑时按列类型弹对应编辑器、数值列自动夹值、空值兜底。诀窍不在委托本身的四个 override（那是 Qt 套路），而在**用回调把列规格喂给委托**，让 detail 层既拿到规格又不依赖父表类型。

## Step 2：detail::ValidatorDelegate + createEditor / setEditorData / setModelData

### 目标

做出一个 `AwesomeQt::detail::ValidatorDelegate : public QStyledItemDelegate`（带 Q_OBJECT）。它通过一个 `ColumnSpecProvider` 回调从父表拉某列的 `{type, min, max, combo}` 规格——`createEditor` 据此挑编辑器（kInt→QSpinBox 设范围、kDouble→QDoubleSpinBox、kCombo→QComboBox 填候选项、kCheck→返回 nullptr 不弹编辑器、kText→QLineEdit）；`setEditorData` 把当前格的值喂进编辑器，**数值列空值兜底成 `box->minimum()`**、kCombo 用 `findText` 回库首项；`setModelData` 提交时直接写 `box->value()` / `combo->currentText()`，不再二次判空。EditableTable 构造时 new 出委托、注入回调、`table_->setItemDelegate(delegate_)`。

### 提示（按顺序）

1. **委托放 detail 子命名空间**：`namespace AwesomeQt::detail { class ValidatorDelegate : public QStyledItemDelegate { Q_OBJECT ... }; }`。Q_OBJECT 类要进 CMake 的源列表（AUTOMOC 已开），否则 moc 不生成元对象
2. **回调类型定义**：`using ColumnSpecProvider = std::function<bool(int column, int& type, double& min, double& max, QStringList& combo)>;`。返回 false 表示该列无规格、退化为文本。注入用 `setColumnSpecProvider(provider)`
3. **createEditor 的 switch**：用 `columnTypeToInt(ColumnType)` 折成 int 做分支（detail 层不依赖枚举）。kInt 分支里 `box->setRange((int)min, (int)max)`——范围用 double 存、整型列截断成 int；kCheck 直接 `return nullptr`（勾选不走委托编辑器）
4. **setEditorData 兜底**：`qobject_cast<QSpinBox*>` 命中后，`value.toInt(&ok)`，`ok ? parsed : box->minimum()`——**空值落回最小值**，不留空。kDouble 同理。kCombo 用 `findText(value)`，找不到回 index 0
5. **setModelData 只管写**：`qobject_cast<QSpinBox*>` 命中后直接 `model->setData(index, box->value(), Qt::EditRole)`——**不判空**。因为 setEditorData 已把空值兜成最小值，SpinBox 又自带夹值，`box->value()` 必然合法
6. **EditableTable 注入回调**：构造里 `delegate_ = new detail::ValidatorDelegate(this)`，`delegate_->setColumnSpecProvider([this](int column, ...){ ... })`——lambda 里查 `columns_` 返回规格。最后 `table_->setItemDelegate(delegate_)`
7. **别忘了 updateEditorGeometry**：`editor->setGeometry(option.rect)`，否则编辑器位置不对

### 关键认知——为什么校验前置、为什么用回调

**校验前置**：直觉是在 setModelData 判空（提交时拦）。但 QSpinBox / QDoubleSpinBox 本身就有 `[min,max]` 夹值——只要进编辑器时空值被兜成最小值，提交时 `value()` 必然在区间内，setModelData 直接写即可，不用再判。校验点收敛到 setEditorData 一处，逻辑更薄、更不容易漏分支。kCombo 同理靠 `findText` 回库兜底。

**用回调而非持父表指针**：委托要知道列规格，最直接的写法是 `EditableTable* owner_` 然后调它的 getter。但这会让 detail 层 include EditableTable 的完整定义、形成「父表持委托、委托持父表」的环。改成吃一个 `std::function` 回调，detail 层只依赖 `int/double/QStringList` 这几个基本类型，连 ColumnType 枚举都不用认识（经 `columnTypeToInt` 折成 int 透传）。耦合降到最低。

### 检查点

双击 Score 列（kInt 0-100）单元格：弹出 QSpinBox（带上下箭头），输 `9999` 回车后格内值变 100（夹值）；清空编辑器再提交，格内值变 0（空值兜底成最小值）。Color 列（kCombo 红/绿/蓝）弹出下拉只能选这三项。Active 列（kCheck）双击**不弹**编辑器，直接点复选框勾选 = 委托分发对了。

> 委托机制不熟？[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。委托 API 细节？[QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html)。

### 对照答案

- ValidatorDelegate 声明 + ColumnSpecProvider 回调类型：`include/editable_table.h:27` / `include/editable_table.h:34`
- createEditor 按列挑编辑器：`src/editable_table.cpp:41`
- setEditorData 空值兜底（kInt/kDouble 落最小值、kCombo findText）：`src/editable_table.cpp:80`
- setModelData 直接写不判空：`src/editable_table.cpp:110`
- 构造注入回调 + setItemDelegate：`src/editable_table.cpp:158` / `src/editable_table.cpp:171`

---

下一步：[Step 3 整表数据往返 + Q_PROPERTY 行为开关](./03-data-roundtrip.md)。
