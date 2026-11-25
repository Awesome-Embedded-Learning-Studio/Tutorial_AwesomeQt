---
title: "卡住怎么办"
description: "按症状查：方块不动/收不到键、旋转错乱、卡墙转不过、I 型格子当空格、连锁消行少消、暂停方块还在掉、高级不可玩、最高分不存、Next 预览乱跳、投影错位、软硬降不计分——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `app/08-games/tetris/`，对照着看。

## 方块不动 / 主窗口装配后收不到方向键

- 棋盘 `setFocusPolicy` 设了吗？子控件默认 `NoFocus`，不设 StrongFocus 主窗口装配后就收不到键。→ `demo/tetris_board.cpp:99`
- 窗口装配后有没有 `board_->setFocus()`？菜单/对话框会抢走焦点，操作完要还回去。→ `demo/tetris_window.cpp:190`、`247`、`253`
- 进阶排查：[事件处理](../../../../../beginner/03-qtwidgets/02-event-handling-beginner.md)

## 旋转后方块翻转方向错乱 / I 型旋转扭曲

- 是不是在运行时算转置（矩阵转置 + 行反转）？方向极易写反。
- 4 旋转态应该全部**预排静态表**（字符串字面量），不在运行时算。→ `demo/tetris_board.cpp:20-92`
- 进阶排查：[自绘控件基础](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)

## 方块贴墙旋转直接失败（卡墙）

- 旋转后撞墙有没有做**壁踢**？硬性回滚就卡死了。
- 壁踢序列：原位 → 左1 → 右1 → 左2 → 右2 → 上1，任一不撞采用，全失败才回滚。→ `demo/tetris_board.cpp:184-202`

## I 型方块锁定后「消失」 / 消行逻辑错乱

- `board_` 是不是直接存了 `type`？I 型 type=0 与「空格 0」撞值。
- 锁定要存 `type + 1`（1..7），0 专属空格；取色时 `type_plus1 - 1`。→ `demo/tetris_board.cpp:256`、`paintEvent` 取色 `429`

## 连锁消行只消了一部分 / 多消了

- `clearFullLines` 是不是自顶向下扫？下移会污染尚未检查的行。必须**自底向上**（r 递减）。→ `demo/tetris_board.cpp:287`
- 一行被上方填满的行下移后**自己又满**（连锁），处理完一行有没有 `++r` 抵消 for 的 `--r` 重检同一位置？→ `demo/tetris_board.cpp:305`

## 暂停后方块还在下落 / 恢复后速度乱了

- `setPaused` 只改了标志位？那 timer 还在走。要在暂停时 `timer_->stop()`、恢复时 `start()`。
- `step` 入口有没有查 `paused_`/`game_over_` 双保险？→ `demo/tetris_board.cpp:331-334`、`356-367`

## 高级等级下方块下落快到不可玩

- `interval` 没下限？高等级 interval 趋近 0。
- `applyLevelSpeed` 要用 `std::max(80, 700 - (level-1)*60)` 兜底 80ms。→ `demo/tetris_board.cpp:340-344`

## 重开程序后最高分丢了 / 显示为 0

- main 里有没有 `setApplicationName`/`setOrganizationName`？QSettings 默认构造靠它们定位存储。→ `demo/main.cpp:13-14`
- board 构造时 `restart()` 会把 high_score_ 清零，窗口构造**后**有没有 `board_->setHighScore(loadHighScore())` 回填？→ `demo/tetris_window.cpp:101-102`
- 落盘有没有挂在 `gameOver` + `closeEvent`？`statsChanged` 只刷面板不写盘（softDrop 高频会狂写），最高分靠 `gameOver` 和 `closeEvent` override 落盘。→ `demo/tetris_window.cpp:107-110`、`262-266`
- 进阶排查：[QSettings 文档](https://doc.qt.io/qt-6/qsettings.html)

## Next 预览画的下一块乱跳 / 形态跟棋盘对不上

- NextPreview 是不是自己重定义了形态？应该复用 `TetrisBoard::shapeOf(type)`，否则棋盘改形态副面板不同步。→ `demo/tetris_window.cpp:53`
- 画前有没有算 4×4 矩阵的包围盒再居中？不同方块格子分布不同，不居中就跳。→ `demo/tetris_window.cpp:54-71`

## 投影画在缝隙形（S/Z）上错位

- 投影是不是按单格逐个算落点？缝隙形上下格落点不同步会错位。
- 投影要按**整块行偏移**算（`ghost_dr` 一路下落直到整块撞底），整块同步移动。→ `demo/tetris_board.cpp:453-457`

## 软降/硬降不计分 / 分数不一致

- `softDrop` 里 `tryMove(1,0)` 成功有没有 `score_ += 1`？→ `demo/tetris_board.cpp:310-317`
- `hardDrop` 有没有 `while(tryMove)` 数 dropped、`score_ += dropped*2`？→ `demo/tetris_board.cpp:319-329`

## 重开（restart）后副面板没刷新 / 显示旧分

- `onRestart` 调完 `board_->restart()` 有没有再 `refreshSidePanel()`？→ `demo/tetris_window.cpp:244-248`
- board 内部 `restart()` 有没有 `emitStats()`（发 statsChanged 让窗口刷）？→ `demo/tetris_board.cpp:135`

## 方块被踢到棋盘顶外（br<0）锁定时部分格子消失 / 堆到顶却没判 game over

- `lockPiece` 写盘前有没有先扫一遍顶出？方块被壁踢上踢、`current_.row + r < 0` 的格子直接 `continue` 丢弃会形态截断 + 漏判结束。→ `demo/tetris_board.cpp:222-242`
- 锁定前任一格 `current_.row + r < 0` 就该判顶出 game over（停钟 + 刷分 + emit gameOver + return），别静默丢弃。
- 对照「出生即碰撞」的另一条 game over 路径：`spawnNext` 出生位碰撞。→ `demo/tetris_board.cpp:145-150`

## P/R 键行为不一致 / offscreen 自动化投键到棋盘不生效

- 是不是 P/R 在 `QAction`（WindowShortcut）+ 棋盘 `keyPressEvent` + 窗口 `keyPressEvent` 三处都绑了？真实 UI 下 QAction 先 accept、棋盘收不到；自动化直投棋盘又走另一条路。
- 棋盘 `keyPressEvent` 保留 `Key_P`（供 offscreen/自动化直投走这条）；窗口 `keyPressEvent` 删掉 `Key_P`/`Key_R` 死代码（不再重复绑，直接 `QMainWindow::keyPressEvent` 兜底）；`Key_R` 仍交 QAction（R 不需 offscreen 测）。→ `demo/tetris_board.cpp:401-403`、`demo/tetris_window.cpp:256-260`

## 狂按 ↓ 时疯狂写盘（CPU/磁盘抖动）/ 中途关窗最高分丢

- `statsChanged` 回调里是不是直接 `saveHighScore` 了？softDrop 每格 +1 都 emit 一次，狂按 ↓ 就狂写 QSettings（无谓 I/O）。→ 别在 `statsChanged` 落盘
- 最高分落盘要推迟到 `gameOver` + `closeEvent`：`statsChanged` 只 `refreshSidePanel` 刷面板，内存里 `board_->highScore()` 是真相源。→ `demo/tetris_window.cpp:106-110`
- 中途关窗会不会来不及落盘？要 override `closeEvent` 在退出时 `saveHighScore`。→ `demo/tetris_window.cpp:262-266`

## moc 报错 / 信号槽不认识

- `TetrisBoard`/`NextPreview`/`TetrisWindow` 头里**有没有 Q_OBJECT**？有自定义信号（`statsChanged`/`gameOver`）就必须有。→ `demo/tetris_board.h:36`、`tetris_window.h:17/36`
- CMake **开了 AUTOMOC 吗**？`qt_add_executable` 默认开，但手写 `add_executable` 要 `set(CMAKE_AUTOMOC ON)`。
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
