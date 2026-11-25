---
title: "Step 2：键盘控制 + 碰撞 + 旋转壁踢 + 锁定 + 消行计分"
description: "QKeyEvent 接方向键、collides 碰撞判定、rotateCurrent 壁踢序列、tryMove 平移下移、lockPiece 烙进 board（存 type+1）、clearFullLines 自底向上消行+连锁重检、计分（100/300/500/800×level）+ 每 10 行升级。"
---

# Step 2：键盘控制 + 碰撞 + 旋转壁踢 + 锁定 + 消行计分

← [手册首页](./index.md) · 上一步 [Step 1 棋盘建模+自绘](./01-board-and-paint.md) · 下一步 [Step 3 副面板+持久化](./03-sidepanel-and-persist.md) →

## Step 2：键盘 + 碰撞 + 旋转壁踢 + 锁定 + 消行 + 计分

### 目标

接上键盘：←→ 移、↑ 旋转、↓ 软降（+1/格）、Space 硬降到底（+2/格）。方块能正确旋转（撞墙自动壁踢、全失败回滚）、能撞底锁定烙进棋盘、消满行（连锁也消）、按消行数计分（1/2/3/4 行 = 100/300/500/800 × 等级）、每 10 行升 1 级下落变快。这一步让游戏**能玩出胜负**——堆到顶 game over。

### 提示

- **碰撞判定 `collides(type, rotation, row, col)`**：遍历 4×4 矩阵的「1」格，换算到棋盘坐标 `(row+r, col+c)`，三件事判撞：①列越界（`bc<0 || bc>=10`）②行下溢出（`br>=20`）③撞已堆格（`board_[br][bc] != 0`）。**关键细节：行允许负**（`br<0` 时 `continue` 不算撞）——方块刚出生可能顶在棋盘上方缓冲区。
- **`tryMove(dRow, dCol)`**：算新位置，`collides` 为真就返回 false 不动，否则更新 row/col + `update()`。←→ 是 `tryMove(0, ±1)`，下落是 `tryMove(1,0)`。
- **旋转 `rotateCurrent` 怎么不卡墙**：算 `new_rot = (rotation+1)%4`，先试原位是否碰撞，碰撞就依次试「左1 → 右1 → 左2 → 右2 → 上1」五个偏移，任一不撞就采用（更新 rotation/row/col + update + return），**全部失败就什么都不做（静默回滚）**。这是简化版壁踢，非 SRS。
- **锁定 `lockPiece`**：遍历当前 4×4 矩阵「1」格，写进 `board_[br][bc]`——**存 `current_.type + 1`**，别存 `type`！因为 I 型 type=0 与「空格 0」撞值，+1 让 0 专属空格。落完调 `clearFullLines` 消行、结算、`spawnNext` 出下一块。
- **消行 `clearFullLines` 怎么做对**：①**自底向上扫**（`r` 从 19 递减）——别自顶向下，下移会污染未检行；②某行满就把 `r..1` 全部下移一格、`board_[0].fill(0)`；③**同位置连锁**：处理完一行后 `++r` 抵消 for 的 `--r`，让下一轮重检同一位置（连锁满行）。
- **计分**：`static const int line_score[] = {0,100,300,500,800}`——下标就是消行数，乘当前 level。`lines_ += cleared`，新等级 = `1 + lines_/10`，变了就 `applyLevelSpeed`。
- **等级 → interval**：`std::max(80, 700 - (level-1)*60)`——每级减 60ms，下限 80ms 防过快不可玩。
- **软降/硬降计分**：`softDrop` 里 `tryMove(1,0)` 成功就 `score_ += 1`，失败（到底）就 `lockPiece`；`hardDrop` 里 `while(tryMove(1,0)) ++dropped` 数下落格数，`score_ += dropped*2`，最后 lockPiece。
- **`spawnNext` 怎么判 game over**：把 next_type_ 提升为当前块、置出生位（`col = 10/2 - 4/2`、`row = 0`），**出生位就碰撞 = 堆顶 = game over**——停钟、`game_over_ = true`、发 `gameOver()` 信号。还有第二条顶出路径在 `lockPiece`：方块被踢到棋盘顶外（`current_.row + r < 0`）锁定时也算 game over——锁定前先扫一遍，任一格在顶外就停钟判负，别把那些格子静默丢弃（否则形态截断 + 漏判结束）。
- **`keyPressEvent`**：先查 `game_over_`（结束就忽略游戏键），switch 各方向键；`Key_P` 在棋盘这里保留（真实 UI 经事件派发链时 QAction 的 WindowShortcut 先 accept，单触发不冲突；offscreen/自动化直投棋盘走这条），`paused_` 时方向键忽略但 P 能恢复。

### 检查点

←/→ 能移、↑ 能转（贴墙也能转过）、↓ 软降加分、Space 硬降到底加分锁定。堆满一行自动消掉、上方下移。**故意堆到顶 → 方块出生位即碰撞 → GAME OVER 停钟**。连消多行分数翻倍（4 连消 800×level）。

> QKeyEvent / 键盘事件不熟？先读 [事件处理](../../../../../beginner/03-qtwidgets/02-event-handling-beginner.md)。
> 信号槽（statsChanged/gameOver）不熟？先读 [信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。

### 对照答案

- `collides`（边界 + 已堆 + 行允许负缓冲）：`demo/tetris_board.cpp:160-182`
- `tryMove`（碰撞不执行返回 false）：`demo/tetris_board.cpp:204-214`
- `rotateCurrent`（壁踢序列 + 全失败回滚）：`demo/tetris_board.cpp:184-202`
- `lockPiece`（存 type+1）：`demo/tetris_board.cpp:219-281`（`256` 是 type+1 那行；`222-242` 是顶出 game over 判定）
- `clearFullLines`（自底向上 + 连锁 ++r）：`demo/tetris_board.cpp:283-308`（`305` 是 ++r）
- 计分 + 升级：`demo/tetris_board.cpp:260-276`
- `applyLevelSpeed`（80ms 下限）：`demo/tetris_board.cpp:340-344`
- `softDrop`/`hardDrop`（计分）：`demo/tetris_board.cpp:310-329`
- `spawnNext`（出生位碰撞 = gameOver）：`demo/tetris_board.cpp:138-151`
- `step`（自然下落到底锁定）：`demo/tetris_board.cpp:331-338`
- `keyPressEvent`（含 `Key_P` 保留）：`demo/tetris_board.cpp:372-408`

---

下一步：接副面板（Next 预览 + 分数面板）+ 菜单 + 暂停/重开 + QSettings 最高分持久化——[Step 3 副面板+持久化](./03-sidepanel-and-persist.md)。
