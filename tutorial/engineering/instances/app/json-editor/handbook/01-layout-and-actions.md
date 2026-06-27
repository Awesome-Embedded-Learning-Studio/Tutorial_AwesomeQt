---
title: "Step 1：中央区分栏 + QAction 装配"
description: "QSplitter 水平拼 QPlainTextEdit 编辑区和 QTreeWidget 树视图当中央区，五个 QAction 复用到菜单栏和工具栏。"
---

# Step 1：中央区分栏 + QAction 装配

← [手册首页](./index.md) · 下一步 [Step 2-3 解析校验与树填充](./02-parse-and-validate.md) →

## Step 1：QSplitter 两栏 + 五个 QAction 装菜单和工具栏

### 目标

窗口中央水平分两栏：左侧 QPlainTextEdit 编辑区（等宽字体、占位提示文字），右侧 QTreeWidget 三列（Key/Index、Value、Type、隔行变色）。顶部菜单栏有 File / JSON / Help 三组，工具栏有 Open / Save / Format / Compact / Validate 五个按钮——**同一个 QAction 在菜单和工具栏都出现**，点哪个都触发同一槽。

### 提示

- `setupCentral`：`new QSplitter(Qt::Horizontal, this)`，左 `QPlainTextEdit`、右 `QTreeWidget`，`addWidget` 进 splitter，`setCentralWidget(splitter)`（splitter 接管两个子部件的所有权）
- 编辑区设等宽字体：`QFontDatabase::systemFont(QFontDatabase::FixedFont)`——取系统默认等宽体，比写死 "Consolas"/"Monaco" 跨平台
- 编辑区 `setPlaceholderText` 给个提示，树 `setHeaderLabels({"Key / Index", "Value", "Type"})` + `setAlternatingRowColors(true)`
- splitter `setStretchFactor(0, 3)` / `setStretchFactor(1, 2)`——窗口拉伸时左 3 右 2 分配多出来的空间
- `setupActions`：五个 `QAction`（Open/Save/Format/Compact/Validate），每个设 `setShortcut`（QKeySequence::Open/Save 是平台标准 Ctrl+O/Ctrl+S，Format/Compact 自定义 Ctrl+I/Ctrl+M，Validate 用 `Qt::CTRL | Qt::Key_Return`）和 `setIcon`（`style()->standardIcon(QStyle::SP_DialogOpenButton)` 等标准图标，不依赖外部资源）
- 每个 action `connect(triggered → this, &slot)`，**menu 和 toolbar 都 add 这同一个 action 指针**——别 new 两份
- **构造顺序**：必须 `setupCentral()` 在 `setupActions()` 之前——actions 的槽会用到 editor_/tree_，先建部件才不会解引用空指针

### 检查点

窗口左右两栏分得开（拖动中间分隔条能调比例），菜单栏三组齐全，工具栏五个按钮，按 Ctrl+O 弹打开对话框（现在槽可以是空的，下一步填）= 装配链通了。

> QSplitter 不熟看 [QSplitter](../../../../../beginner/03-qtwidgets/42-qsplitter-beginner.md)，QPlainTextEdit 看 [QPlainTextEdit](../../../../../beginner/03-qtwidgets/24-qplaintextedit-beginner.md)，QTreeWidget 看 [QTreeWidget](../../../../../beginner/03-qtwidgets/48-qtreewidget-beginner.md)。QAction 装配看 [菜单栏/菜单/Action](../../../../../beginner/03-qtwidgets/56-qmenubar-menu-action-beginner.md) 和 [工具栏](../../../../../beginner/03-qtwidgets/57-qtoolbar-beginner.md)。

### 对照答案

- 中央区 splitter + 等宽字体 + 树表头：`demo/json_editor_window.cpp:55-73`
- 五个 QAction 装配（快捷键 + 标准图标 + connect）：`demo/json_editor_window.cpp:78-103`
- 菜单栏 File/JSON/Help 三组复用 action：`demo/json_editor_window.cpp:105-124`
- 工具栏复用同一组 action：`demo/json_editor_window.cpp:126-136`
- 构造函数顺序（central 先于 actions）：`demo/json_editor_window.cpp:42-46`

---

下一步是重头戏：[Step 2-3 解析校验 + 树递归填充](./02-parse-and-validate.md)。
