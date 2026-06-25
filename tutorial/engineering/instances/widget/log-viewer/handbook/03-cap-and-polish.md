---
title: "Step 6：行数上限裁旧 + 收尾"
description: "用 document 原生 blockCount 判定超限，trimOldBlocks 从文档头按块连选删除；setMaxLines 改上限后立即裁一次当场收敛；补 lineCount / clear / sizeHint 收尾。"
---

# Step 6：行数上限裁旧 + 收尾

← [Step 4-5](./02-autoscroll-and-trim.md) · [手册首页](./index.md) →

## Step 6：行数上限裁旧（maxLines + trimOldBlocks）

### 目标

实现 `trimOldBlocks()`：当前 document 块数超过 `max_lines_` 时，从文档头连选删除多出来的块。`append` 每次写完行后调一次；`setMaxLines` 改上限后也立即调一次，让控件状态当场收敛。

### 提示

- **不要自维护行计数器**——直接用 `view_->blockCount()`，让 document 当唯一真相源，省掉 clear 后清零、和实际块数漂移这些坑
- `trimOldBlocks`：`const int count = view_->blockCount();`；`if (count <= max_lines_) return;`；`const int toRemove = count - max_lines_;`
- 删除：`QTextCursor cursor(view_->document()); cursor.movePosition(QTextCursor::Start);`
- 从文档头连续选中 `toRemove` 个块（含每块的行尾换行）：循环里 `movePosition(NextBlock, KeepAnchor)` 选到下一块头，再 `movePosition(NextCharacter, KeepAnchor)` 把行尾 `\n` 带进选区
- 循环结束 `cursor.removeSelectedText()` 一次性删掉整段
- `setMaxLines` 在 clamp 完、emit 之前调一次 `trimOldBlocks()`——保证上限一改、内容当场符合新约束

关键认知——**为什么用块而非字符删**：每条日志是独立一块（step 3 那个 `\n` 保证的），按块删语义干净、不会把半行切掉；而且 `blockCount` 是 QPlainTextEdit 原生维护的，比自己数 `\n` 靠谱。

### 检查点

默认 `maxLines=1000`，连发 200 条 Info（demo 里 Burst 200 Info 按钮）后，document 块数稳定在 200（远没到上限）；把 `setMaxLines` 调到 100 再连发 200 条，块数稳定在 100 附近、顶部旧行被裁、底部始终是最新那条；`setMaxLines(50)` 当场就把现有内容裁到 50 = 裁旧对了。

> 对象生命周期 / 富文本操纵不熟？[内存管理进阶](../../../../../advanced/01-qtbase/06-memory-management-advanced.md)、[QPlainTextEdit 入门](../../../../../beginner/03-qtwidgets/24-qplaintextedit-beginner.md)。

### 对照答案

- trimOldBlocks 块级删除（blockCount 判定 + Start 连选 + removeSelectedText）：`src/log_viewer.cpp:154`
- setMaxLines 即时裁旧：`src/log_viewer.cpp:93`
- append 末尾调 trimOldBlocks：`src/log_viewer.cpp:56`

---

## 收尾：lineCount / clear / sizeHint

### 目标

补三个小接口让控件完整可用：`int lineCount() const` 供 demo 观察裁旧行为；`void clear()` 一键清空；`QSize sizeHint() const override` 给布局一个建议尺寸。

### 提示

- `lineCount`：直接 `return view_->blockCount();`——薄透传，别自己计数
- `clear`：`view_->clear();`——view 自带，清完 document 块数归零，trimOldBlocks 下次自然不会误删
- `sizeHint`：返回 `QSize(380, 220)` 这种合理默认，让控件放进布局有个起手尺寸

### 检查点

demo 里行数回显跟着 `lineCount()` 变化、连发压测时能看到块数收敛在上限；Clear 按钮一按文本框空、行数归零；控件单独放进布局有个合适起手大小 = 收尾对了。

### 对照答案

- lineCount 透传：`src/log_viewer.cpp:81`
- clear 透传：`src/log_viewer.cpp:77`
- sizeHint：`src/log_viewer.cpp:125`

---

搓完了。跑 demo 对照成品：三级染色 + 自动滚底 + 连发压测裁旧 + Clear 都能复现 = 你搓的和 repo 一致。

想再深？回 [手册首页](./index.md) 看进阶挑战（级别过滤 / 搜索高亮 / 多通道分流 / model 后端虚拟化）。
