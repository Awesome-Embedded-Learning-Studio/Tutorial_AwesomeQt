---
title: "Step 3：批量方法 blockSignals 守卫 + Q_PROPERTY 收尾"
description: "checkAll/uncheckAll/invertChecked/addItems 整段 blockSignals 防 itemChanged 回灌雪崩；alternatingRowColors/spacing 两个 Q_PROPERTY 收尾；setItemChecked 单项故意放行。"
---

# Step 3：批量方法 blockSignals 守卫 + Q_PROPERTY 收尾

← [Step 2](./02-itemchanged-and-collect.md) · [手册首页](./index.md) →

这一步给控件补上批量操作能力——`checkAll`/`uncheckAll`/`invertChecked` 三键，外加批量 `addItems`。难点不在循环本身，而在「批量 `setCheckState` 每项都回灌 `itemChanged`」这颗雷，整段用 `blockSignals` 守住。再补两个 Q_PROPERTY 收尾。

## Step 3：批量方法 blockSignals 守卫 + Q_PROPERTY 收尾

### 目标

加三个批量方法（全选/全不选/反选）+ 批量 `addItems`，每个都在循环前后整段 `blockSignals(true/false)` 守卫，防 `itemChanged` 回灌成信号洪流；`setItemChecked` 单项故意不守卫允许透传；补 `alternatingRowColors`/`spacing` 两个 Q_PROPERTY，setter 带无变化早返回 + spacing 负值 clamp。

### 提示（按顺序）

1. **`checkAll()` / `uncheckAll()`**：先 `if (list_ == nullptr) return;`，再 `const bool was_blocked = list_->blockSignals(true);` 保存旧值；`for (int i = 0; i < list_->count(); ++i)` 遍历，每项 `item->setCheckState(Qt::Checked)`（或 Unchecked），`item != nullptr` 判空；循环后 `list_->blockSignals(was_blocked)` 恢复
2. **`invertChecked()`**：同样守卫，但循环里要先读旧态 `const bool on = item->checkState() == Qt::Checked;` 再写 `item->setCheckState(on ? Qt::Unchecked : Qt::Checked);`——读和写都在屏蔽期内完成，否则回灌的 `itemChanged` 会干扰下一项的读
3. **`addItems(const QStringList& texts)`**：批量初始化版，整段 `blockSignals` 守卫，循环里 `new QListWidgetItem` + `setFlags(ItemIsUserCheckable)` + `setCheckState(Unchecked)` + `addItem`
4. **`setItemChecked(QListWidgetItem* item, Qt::CheckState state)`**：入口 `if (item == nullptr) return;`，然后直接 `item->setCheckState(state)`——**故意不守卫**，允许 `checkedChanged` 透传，外部程序化改单项时就靠它拿通知
5. **Q_PROPERTY `alternatingRowColors`（bool）**：READ `list_->alternatingRowColors()`、WRITE 里 `if (list_->alternatingRowColors() == enabled) return;` 无变化早返回、再 `setAlternatingRowColors(enabled)` + `emit alternatingRowColorsChanged(enabled)`
6. **Q_PROPERTY `spacing`（int）**：WRITE 入口先 `if (list_ == nullptr || pixels < 0) return;`（负值 clamp），再判无变化早返回，最后 `setSpacing(pixels)` + `emit spacingChanged(pixels)`
7. **`sizeHint()`** override 返回 `{200, 240}`，给布局稳定建议尺寸

### 关键认知

- **批量 vs 单项的守卫策略正好相反**：批量方法（三键 + addItems）整段守卫、不透传逐项信号；`setItemChecked` 单项故意放行。为什么？批量改写时外部通常不需要逐项通知（按完按钮自己会去读结果），逐项回灌纯粹是噪音 + 性能塌陷；而单项程序化改写时，外部恰恰要靠 `checkedChanged` 知道「这一项被改了」。这是 checkbox-tree 双闸门教训的列表简化版——没有递归就不需要 `is_propagating_` 标志位那道额外闸门
- **blockSignals 要保存恢复旧值而非硬置 false**：用 `was_blocked` 模式（`const bool was_blocked = list_->blockSignals(true); ...; list_->blockSignals(was_blocked);`）。硬置 false 可能误伤别的信号连接——万一进入前 `list_` 上已有别的被 block 状态，硬清就破坏了调用方的守卫契约
- **invertChecked 的读旧态必须在屏蔽期内**：反选是「读旧、写反」两步，若不挡信号，写新态触发的 `itemChanged` 回灌会干扰下一次 `checkState()` 的读——读到的可能是被回灌改过的值，反选结果错乱
- **Q_PROPERTY 无变化早返回是卫生习惯**：外部反复 set 同值，setter 入口先判相等 return，避免发一堆空 NOTIFY。spacing 还多一道负值 clamp，负行距无意义

### 检查点

按 `Check all`/`Uncheck all`/`Invert selection` 各一次，列表整体翻动平滑无卡顿 = 批量守卫生效；连 `checkedChanged` 的槽在批量操作时**不被**连续刷屏（雪崩挡住了）；`setItemChecked(item, Checked)` 单项调用时槽**能**收到通知（单项放行）；翻 `alternatingRowColors` 斑马纹显隐 = 属性开关对了；`setSpacing` 传负值被挡掉、传同值不发重复 NOTIFY = clamp + 早返回生效；`checkAll` 后调 `checkedTexts()` 返回全部项 = 批量 + 汇总联动通了。

> blockSignals 语义看 [QObject::blockSignals](https://doc.qt.io/qt-6/qobject.html#blockSignals)；Q_PROPERTY 机制不熟读 [QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)。对照 checkbox-tree 的双闸门升级版：[checkbox-tree 成品导览](../../checkbox-tree/)。

### 对照答案

- checkAll/uncheckAll 整段守卫：`src/checkbox_list.cpp:63` / `:78`
- invertChecked 读旧态写反态（屏蔽期内）：`src/checkbox_list.cpp:92-107`
- addItems 批量初始化守卫：`src/checkbox_list.cpp:41-51`
- setItemChecked 单项不守卫（边界 + 放行）：`src/checkbox_list.cpp:53-61`
- alternatingRowColors Q_PROPERTY：`include/checkbox_list.h:35` / setter `src/checkbox_list.cpp:148-157`
- spacing Q_PROPERTY（负值 clamp + 无变化早返回）：`include/checkbox_list.h:37` / setter `src/checkbox_list.cpp:163-172`
- sizeHint：`src/checkbox_list.cpp:174-176`

---

成品全部要素都齐了。回到 [手册首页](./index.md) 看进阶挑战（升级成 checkbox-tree 父子联动 / 换 QListView+model / 持久化勾选集），或回 [成品导览](../) 对照完整实现。
