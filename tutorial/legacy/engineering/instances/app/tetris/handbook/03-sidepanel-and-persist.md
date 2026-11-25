---
title: "Step 3：副面板 + 菜单 + 暂停 + 最高分持久化"
description: "NextPreview 自绘下一块预览（复用 TetrisBoard 形态表 + 包围盒居中）、QHBoxLayout 棋盘+副面板装配、QMenuBar Game/Help 菜单、QSettings 最高分持久化（构造后回填 + statsChanged 落盘）、键盘焦点管理、statsChanged 信号驱动副面板刷新。"
---

# Step 3：副面板 + 菜单 + 暂停 + 最高分持久化

← [手册首页](./index.md) · 上一步 [Step 2 键盘+碰撞+消行](./02-control-lock-and-clear.md) →

## Step 3：Next 预览副面板 + 菜单 + 暂停/重开 + QSettings 最高分

### 目标

棋盘右边加一块副面板：Next 预览（下一块形态）、SCORE/LEVEL/LINES/BEST、操作说明。加 Game 菜单（Restart/Pause/Quit）和 Help 菜单（About）。P 暂停、R 重开。最高分用 QSettings 持久化——重开程序还在。至此整机成品成型：**自绘画布 + 节拍循环 + 键盘 + 副面板状态同步 + 持久化**一应俱全。

### 提示

- **NextPreview 怎么画才不跳**：别自己重定义形态——直接调 `TetrisBoard::shapeOf(type).rotations[0]` 取 4×4 矩阵和颜色。画之前先扫矩阵算「1」格的 min/max row/col（包围盒），再按包围盒居中，否则不同方块在框里位置乱跳。
- **副面板装配**：主窗口 `setupCentral` 里 `QHBoxLayout`——左边 `board_`（stretch=1 占主区），右边一个 `QWidget` 套 `QVBoxLayout` 堆 NEXT 标题 + NextPreview + SCORE/LEVEL/LINES/BEST + stretch + CONTROLS 说明。
- **SCORE/LEVEL/LINES/BEST 行怎么造**：写个 `make_row` lambda——标题 QLabel（粗体）+ 数值 QLabel（大字号）竖排，省得四行重复代码。
- **状态同步靠信号**：棋盘 `statsChanged()` 一发，主窗口 `refreshSidePanel()` 刷四个数值 + NextPreview + 状态栏文字（Playing/Paused/GAME OVER）。`gameOver()` 信号触发落最高分 + 刷面板。
- **QSettings 最高分**：定义键 `"tetris/highscore"`。①`loadHighScore` 用 `QSettings().value(key, 0)` 读；②**关键时序**：`board_->setHighScore(loadHighScore())` 必须在 board 构造**之后**调——因为 board 构造时 `restart()` 把 high_score_ 清零了，得回填；③**落盘别挂在 statsChanged**：softDrop 每格 +1 都 emit 一次 `statsChanged`，狂按 ↓ 就会狂写 QSettings（无谓 I/O）。落盘推迟到 `gameOver` + `closeEvent`——`statsChanged` 回调只 `refreshSidePanel` 刷面板，内存里 `board_->highScore()` 才是真相源。`closeEvent` 要 override，保中途关窗不丢分。
- **main.cpp 要设什么**：`app.setApplicationName("...")` + `app.setOrganizationName("...")`——QSettings 默认构造靠这两个定位存储位置，不设的话持久化可能写到奇怪的地方。
- **菜单**：`menuBar()->addMenu("&Game")` 加 Restart（Ctrl+R 其实直接 `setShortcut(Qt::Key_R)`）、Pause（Key_P）、Quit（`QKeySequence::Quit`）。Help 菜单加 About（`QMessageBox::about`）。
- **键盘焦点管理**：每个会抢焦点的操作（点菜单、点窗口）之后，记得 `board_->setFocus()` 把焦点还回棋盘，否则下一次方向键又收不到。`onRestart`/`onPauseToggle` 末尾都调一次。
- **主窗口 keyPressEvent 不绑 R/P**：R/P 由 Game 菜单的 `QAction`（WindowShortcut）统一处理，窗口 `keyPressEvent` 里不再重复绑定（直接 `QMainWindow::keyPressEvent(event)` 兜底冒泡）；棋盘自己 `keyPressEvent` 保留 `Key_P`（供 offscreen/自动化直投棋盘走），`Key_R` 仍交 QAction。这样避免三处（QAction + 棋盘 + 窗口）重复绑定导致单键双触发或漏触发。

### 检查点

跑起来 → 右边副面板有 NEXT 预览（与实际下一块一致、位置不跳）+ 四个分数面板 + 操作说明。打几分 → 分数面板实时刷新。P 暂停 → 棋盘遮罩「PAUSED」、状态栏「Paused」、计时器真停（方块不动）。R 重开 → 棋盘清空、分数归零。**故意打高分后退出重开程序 → BEST 还在**（持久化通了）。

> QSettings 持久化不熟？先读 [QSettings 配置持久化](https://doc.qt.io/qt-6/qsettings.html)（应用名/组织名与键的关系）。
> 菜单/工具栏不熟？先读 [菜单栏与 Action](../../../../../beginner/03-qtwidgets/56-qmenubar-menu-action-beginner.md)。
> 布局不熟？先读 [布局系统](../../../../../beginner/03-qtwidgets/01-layout-system-beginner.md)。
> QMessageBox 不熟？先读 [QMessageBox 对话框](../../../../../beginner/03-qtwidgets/62-qmessagebox-beginner.md)。

### 对照答案

- `NextPreview::paintEvent`（复用 shapeOf + 包围盒居中）：`demo/tetris_window.cpp:46-88`（`53` 取矩阵、`54-64` 算包围盒、`68-71` 算偏移居中）
- `setupCentral`（QHBoxLayout 棋盘 + 副面板）：`demo/tetris_window.cpp:115-191`
- `make_row` lambda（造 SCORE/LEVEL/LINES/BEST）：`demo/tetris_window.cpp:145-157`
- 操作说明列表：`demo/tetris_window.cpp:169-179`
- `statsChanged` → 只刷副面板（不落盘）：`demo/tetris_window.cpp:106`
- `gameOver` → 落最高分 + 刷面板：`demo/tetris_window.cpp:107-110`
- `refreshSidePanel`（四数值 + Next + 状态栏文字）：`demo/tetris_window.cpp:217-232`
- `loadHighScore`/`saveHighScore`：`demo/tetris_window.cpp:234-242`
- 构造后回填 high_score_：`demo/tetris_window.cpp:101-102`
- `setupMenuBar`（Game/Help 菜单）：`demo/tetris_window.cpp:193-215`
- `setApplicationName`/`setOrganizationName`：`demo/main.cpp:13-14`
- 主窗口 `keyPressEvent`（已不绑 R/P，全交 QAction）：`demo/tetris_window.cpp:256-260`
- `closeEvent`（退出时落盘最高分）：`demo/tetris_window.cpp:262-266`
- `onRestart`/`onPauseToggle` 末尾 `setFocus`：`demo/tetris_window.cpp:244-254`

---

整机成品成型。想再深一层（真 SRS、7-bag 出块、Hold 槽、音效、T-spin）回 [手册首页](./index.md) 看进阶挑战，或对照 [成品导览](../) 的踩坑表查漏。
