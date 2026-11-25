---
title: "Step 1：棋盘建模 + 自绘渲染 + 自然下落"
description: "10×20 棋盘数据建模、7 形态方块 4×4 包络预排 4 旋转态、QPainter 逐格自绘（网格+方块+边框）、QTimer 驱动方块自然下落。"
---

# Step 1：棋盘建模 + 自绘渲染 + 自然下落

← [手册首页](./index.md) · 下一步 [Step 2 键盘+碰撞+消行](./02-control-lock-and-clear.md) →

## Step 1：棋盘数据建模 + 7 形态方块 + QPainter 自绘 + QTimer 下落

### 目标

窗口里画出一个黑底棋盘（10 列 × 20 行的网格），中间掉一个固定形态的方块（先不旋转、不控制），它随 QTimer 每隔一段时间自动下落一格，到底就停住——**能看到「方块在动」**。这一步先把「数据建模 + 自绘 + 节拍」三件事走通，键盘和游戏规则下一 step 才上。

### 提示

- **棋盘数据建模**：`std::array<std::array<int, kBoardCols>, kBoardRows> board_`——20 行 × 10 列，0 表示空格。常量 `kBoardCols=10`/`kBoardRows=20`/`kPieceMatrix=4`（4×4 包络边长）。
- **方块用一个结构体表示当前下落中的方块**：`struct ActivePiece { int type, rotation, row, col; }`——type 0..6（I/O/T/S/Z/J/L），row/col 是 4×4 包络的左上角在棋盘上的坐标。
- **7 种方块怎么定义？** 别手填 0/1 矩阵——用一个 `mk` lambda 把 `"####"`/`".##."` 这样的字符串字面量翻成 4×4 int 矩阵（`#`=占），每种 4 个旋转态全部预排好。这一步可以先只填 1 种（比如 O 型）跑通，7 种全填等下一 step。
- **`paintEvent` 自绘顺序**：1) 算画布偏移让棋盘水平/垂直居中 2) 画已堆方块（board_ 里非 0 的格）3) 画网格线 + 边框 4) 画当前下落方块（按 currentMatrix 的「1」格画）。先不画投影和遮罩。
- **格子怎么画有体积？** 别只 `fillRect`——主色填完后，左/上画一条 `base.lighter(160)` 的高光、右/下画一条 `base.darker(160)` 的阴影，立体感立刻出来。
- **QTimer 怎么驱动下落**：构造时 `timer_ = new QTimer(this)`，`setInterval(700)`（700ms/格起步），`connect(timeout, step)`；`step` 里调 `tryMove(1,0)` 下移一格，**这一步可以先不实现 tryMove 的碰撞返回，到底就把 row 卡住或简单循环不动的占位**——下一 step 才接真正的 lockPiece。
- **键盘焦点**：构造时 `setFocusPolicy(Qt::StrongFocus)`——不设的话主窗口装配后棋盘收不到键（这一 step 还没键盘，但先种下）。

### 检查点

跑起来 → 窗口里一个黑底棋盘网格 → 一个方块从顶部出现、每隔 ~700ms 往下掉一格 → 到底停住。**能看到方块在节拍下下落** = 数据建模 + 自绘 + QTimer 三件套通了。

> QPainter 自绘不熟？先读 [QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md)。
> QTimer 不熟？先读 [QTimer 定时器](../../../../../beginner/01-qtbase/11-timer-beginner.md)。
> 自定义控件 paintEvent 不熟？先读 [自绘控件基础](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)。

### 对照答案

- 常量定义（kBoardCols/kBoardRows/kPieceMatrix）：`demo/tetris_board.h:17-22`
- `mk` lambda 字符串字面量 → 4×4 矩阵：`demo/tetris_board.cpp:22-31`
- 7 种方块预排 4 旋转态（颜色按经典约定）：`demo/tetris_board.cpp:33-90`
- `currentMatrix` 按 type/rotation 取当前 4×4 矩阵：`demo/tetris_board.cpp:156-158`
- `paintEvent` 自绘顺序（已堆→网格→边框→当前方块）：`demo/tetris_board.cpp:413-484`
- `drawBlock` 立体感（高光左上 + 阴影右下）：`demo/tetris_board.cpp:502-519`
- QTimer 装配（700ms 起步 + connect timeout）：`demo/tetris_board.cpp:103-105`
- `step` 下落到底锁定（占位可看完整逻辑）：`demo/tetris_board.cpp:331-338`
- `setFocusPolicy(StrongFocus)`：`demo/tetris_board.cpp:99`

---

下一步：接上键盘控制（移/旋转/软降/硬降）+ 碰撞判定 + 旋转壁踢 + 锁定 + 消行计分——[Step 2 键盘+碰撞+消行](./02-control-lock-and-clear.md)。
