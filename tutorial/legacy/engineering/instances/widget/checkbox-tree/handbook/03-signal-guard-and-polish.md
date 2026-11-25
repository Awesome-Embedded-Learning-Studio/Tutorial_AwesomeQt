---
title: "Step 3：双闸门防信号雪崩 + Q_PROPERTY 联动开关收尾"
description: "is_propagating_ 标志 + blockSignals 守卫双重防 itemChanged 雪崩；propagationEnabled 属性关掉联动退化普通勾选树；API 收尾。"
---

# Step 3：双闸门防信号雪崩 + Q_PROPERTY 联动开关收尾

← [Step 2](./02-propagation-and-recalc.md) · [手册首页](./index.md) →

Step 2 你大概率已经撞上「勾一下就崩」——那是 itemChanged 在递归改子项时被反复触发的信号雪崩。这一步把它彻底按住，再给控件加个联动开关收尾。

## Step 3：双闸门防信号雪崩 + Q_PROPERTY 收尾

### 目标

加两道闸门让程序化 setCheckState 不再引发 itemChanged 回环（栈溢出/性能塌陷）；再补 `propagationEnabled` 属性，关掉就退化成普通勾选树；最后把 `setItemChecked`/`checkAll`/`uncheckAll`/`checkedItems` 这些 API 补齐。

### 提示（按顺序）

1. **闸门一：成员标志 `is_propagating_`（默认 false）**
   - `onItemChanged` 入口先判 `if (is_propagating_) return;`——自己程序化触发的修改直接跳过，不再二次递归
   - 在 `onItemChanged` 真正干活前置 `is_propagating_ = true`，干完置回 `false`
2. **闸门二：`tree_->blockSignals(true/false)` 守卫批量改写**
   - `propagateDown`、`recalcUp`、`checkAll` 这些批量 setCheckState 的地方，调用前后包 `blockSignals`，彻底切断信号回环
   - 用「保存旧值、改完恢复」的写法：`const bool was = tree_->blockSignals(true); ...; tree_->blockSignals(was);`，别硬写成 false 把别的信号也永久挡了
3. **两道闸冗余但都留**：单靠 `is_propagating_` 入口挡，万一某条路径漏置标志就裸奔；单靠 blockSignals 又挡不住「外部直接 setCheckState 后用户再点」的混合触发。两道一起最稳
4. **Q_PROPERTY `propagationEnabled`**：READ/WRITE/NOTIFY 三件，`setPropagationEnabled` 里先判 `if (propagation_enabled_ == enabled) return;` 去重再 emit
5. **联动关闭分支**：`onItemChanged` 和 `setItemChecked` 开头判 `if (!propagation_enabled_)`，只改自身 + emit signal，短路掉 propagateDown + recalcUp
6. **API 补齐**：`checkedItems()` 从顶层逐棵深度优先收 Checked（写个匿名命名空间的 `collectChecked` 递归辅助，别用 `QTreeWidgetItemIterator` 默认构造——坑见 troubleshooting）；`setItemChecked(item, state)` 走完整联动逻辑（非裸 setCheckState）；`checkAll/uncheckAll` 顶层逐项置 Checked/Unchecked 再联动

### 关键认知

- **程序化 setCheckState 一定会再触发 itemChanged**：这是 Qt Item View 的既定行为，不是 bug。你改一个子项 → view 发 itemChanged → 你的槽又被调 → 又改子项……无限递归。不挡必崩
- **blockSignals 要保存恢复旧值而非硬置 false**：硬写 false 可能误伤别的信号连接，`was_blocked` 模式才安全
- **is_propagating_ 的边界**：它只在「本控件自己发起的程序化修改」期间为 true，用户点击触发的 itemChanged 进来时它是 false——靠这个区分「谁触发的」。设置时机要精确包住整段批量修改

### 检查点

快速反复勾选顶层项不崩、不卡 = 双闸门生效；翻 `propagationEnabled` 关掉联动后勾父项子项不动 = 联动开关对了；`checkAll` 后所有项（含深层子孙）全勾、再 `uncheckAll` 全清 = 批量 API 走通；空树调 `checkedItems()` 返回空、`setItemChecked(nullptr, ...)` 不崩 = 边界安全。

> blockSignals 语义看 [QObject::blockSignals](https://doc.qt.io/qt-6/qobject.html#blockSignals)；Q_PROPERTY 机制不熟读 [QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)、进阶 [属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

### 对照答案

- is_propagating_ 入口闸门：`src/checkbox_tree.cpp:183`（onItemChanged）/ `:92`（声明）
- blockSignals 守卫批量改写：`src/checkbox_tree.cpp:197`（propagateDown 前）、`:228`（recalcUp 每层）、`:103`（setItemChecked）
- propagationEnabled Q_PROPERTY：`include/checkbox_tree.h:30` / setter `src/checkbox_tree.cpp:155`
- 联动关闭分支：`src/checkbox_tree.cpp:92-99`（setItemChecked）、`:187-190`（onItemChanged）
- collectChecked 匿名命名空间辅助（避开迭代器默认构造）：`src/checkbox_tree.cpp:16`
- checkAll/uncheckAll 批量：`src/checkbox_tree.cpp:113` / `:132`

---

成品全部要素都齐了。回到 [手册首页](./index.md) 看进阶挑战（换 QTreeView+model / 惰性联动 / 持久化勾选集），或回 [成品导览](../) 对照完整实现。
