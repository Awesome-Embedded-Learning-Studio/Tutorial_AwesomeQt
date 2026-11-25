---
title: "Step 4-5：同目录翻页与幻灯片"
description: "rebuildDirList 记目录图片列表，navigate 循环翻页且跳过坏图；QTimer 驱动幻灯片全屏，进出恢复窗口状态。"
---

# Step 4-5：同目录翻页与幻灯片

← [手册首页](./index.md) · 上一步 [Step 2-3 缩放旋转](./02-zoom-and-rotate.md) →

## Step 4：同目录翻页（循环、跳坏图）

### 目标

打开一张图后，← / → 或工具栏能在同目录的图片间翻页，循环到头绕回，遇到坏图自动跳过不卡。

### 提示

- 打开成功后 `rebuildDirList`：用 `QDir` 列出同目录、按名字排序、过滤图片格式，`indexOf` 定位当前游标 `current_index_`
- `navigate(offset)`：沿 offset 方向**循环**找下一张；`idx = (current_index_ + offset*step) % n`，负数 `+n` 兜底
- **关键**：`loadImage` 返回 true 才提交 `current_index_`——坏图加载失败不污染游标，否则「游标指坏图、画面停旧图」脱钩
- 单张 / 空目录时 `prev/next` action `setEnabled(false)`
- 翻页保持当前 zoom/rotation（看图器惯例），用户要复位自己点 Fit / 100%

### 检查点

目录放 3 张图 + 1 张坏图，翻页能循环、**坏图被跳过**、游标和显示始终对得上 = 翻页稳了。

### 对照答案

- rebuildDirList 列目录 + 定位游标：`demo/image_viewer_window.cpp:207-216`
- navigate 循环跳坏图、成功才提交游标：`demo/image_viewer_window.cpp:223-246`

## Step 5：幻灯片全屏 + 窗口状态恢复

### 目标

F5 进全屏幻灯片，3 秒自动翻页，Esc / Space 退出，退出后窗口恢复进全屏前的状态（最大化的不丢）。

### 提示

- `QTimer` 3000ms，`timeout` 连 `navigate(+1)`；手动按 ← / → 翻页时 `timer->start()` 重置倒计时
- 进全屏前记 `was_maximized_ = isMaximized()`，再 `showFullScreen()`、藏 menuBar / 工具栏
- 退出时 `showNormal()` 会丢最大化——按 `was_maximized_` 决定 `showMaximized()` 或 `showNormal()`
- keyPressEvent：Esc / Space 在幻灯片态退出；← / → 翻页（两种态都翻）
- 坏图在幻灯片里不能弹模态框（打断循环）——loadFailed 槽在幻灯片态只刷状态栏

### 检查点

F5 全屏自动翻页；← / → 手动翻重置计时；Esc 退出后窗口大小 / 最大化态和进之前一致 = 幻灯片通了。

> QTimer 不熟？[定时器基础](../../../../../beginner/01-qtbase/11-timer-beginner.md)。QMainWindow 看 [QMainWindow 主窗口](../../../../../beginner/03-qtwidgets/55-qmainwindow-beginner.md)。

### 对照答案

- 幻灯片 timer 驱动：`demo/image_viewer_window.cpp:38-40`
- 进出幻灯片 + was_maximized_：`demo/image_viewer_window.cpp:293-321`
- loadFailed 幻灯片静默：`demo/image_viewer_window.cpp:170-176`

---

整机搓完了。回头读一遍 [成品导览](../) 的架构图，对照看自己搓的和成品的差异。
