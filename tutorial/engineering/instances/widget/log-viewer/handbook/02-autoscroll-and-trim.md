---
title: "Step 4-5：行为开关 Q_PROPERTY + 自动滚底"
description: "把 maxLines / autoScroll / showTimestamp 升级为 Q_PROPERTY 三件套，append 末尾按 autoScroll 决定要不要 moveCursor(End) + ensureCursorVisible 滚底。"
---

# Step 4-5：行为开关 Q_PROPERTY + 自动滚底

← [Step 1-3](./01-compose-and-append.md) · 下一步 [Step 6 行数上限裁旧](./03-cap-and-polish.md) →

## Step 4：行为开关升级为 Q_PROPERTY

### 目标

把三个开关成员 `max_lines_` / `auto_scroll_` / `show_timestamp_` 从裸成员升级成完整 Q_PROPERTY：每个都有 `Q_PROPERTY(... READ ... WRITE ... NOTIFY ...)`、对应的 getter / setter / signal。`append` 里读 `show_timestamp_` 决定要不要加时间戳前缀。

### 提示

- 三个 `Q_PROPERTY` 放 `Q_OBJECT` 之后：`maxLines`(int, 默认 1000) / `autoScroll`(bool, 默认 true) / `showTimestamp`(bool, 默认 true)
- 每个 setter 走「无变化 early-return」：`if (newVal == member_) return;`——避免无谓的 NOTIFY 触发
- setter 模板：改成员 → （maxLines 这步先不管裁旧，留到 step 6）→ emit signal
- `show_timestamp_` 默认 true；`append` 里 `if (show_timestamp_)` 包住时间戳拼装那段
- `setMaxLines` 要把入参夹到 `>= 1`，避免上限 0 时裁成空或被绕过

### 检查点

外部 `setShowTimestamp(false)` 后再 append，行头没有时间戳了；`setShowTimestamp(true)` 恢复。`setMaxLines(0)` 不会把上限设成 0（被夹成 1）。每个 setter 重复 set 同值不 emit signal = 属性化对了。

> Q_PROPERTY / NOTIFY 不熟？[属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

### 对照答案

- 三个 Q_PROPERTY：`include/log_viewer.h:29-32`
- setShowTimestamp setter + early-return：`src/log_viewer.cpp:113-119`
- setMaxLines clamp：`src/log_viewer.cpp:85-95`

---

## Step 5：自动滚底（autoScroll 开关）

### 目标

`append` 写完行、做完裁旧之后，如果 `auto_scroll_` 为真，就滚到文档末尾让最新行可见。`autoScroll` 关了就不滚，方便用户往上翻看历史时不被打断。

### 提示

- `append` 末尾：`if (auto_scroll_) { view_->moveCursor(QTextCursor::End); view_->ensureCursorVisible(); }`
- **两件套缺一不可**：只 `moveCursor(End)` 把光标挪到文末，但视口不一定跟着滚；`ensureCursorVisible()` 才保证光标所在行进入可视区
- 注意滚底在裁旧（step 6）之后调——先裁后滚，滚到的才是最终末尾

### 检查点

连发几条 append，每次新行都自动出现在视口底部；勾掉 Auto Scroll（调 `setAutoScroll(false)`）再 append，视口不跟着滚、光标停原处，可以往上翻看 = 滚底开关对了。

> 事件循环 / 视口滚动不熟？[事件系统进阶](../../../../../advanced/01-qtbase/07-event-system-advanced.md)。

### 对照答案

- 滚底两件套（auto_scroll_ 门控）：`src/log_viewer.cpp:58-62`

---

下一步：[Step 6 给控件装上行数上限裁旧](./03-cap-and-polish.md)。
