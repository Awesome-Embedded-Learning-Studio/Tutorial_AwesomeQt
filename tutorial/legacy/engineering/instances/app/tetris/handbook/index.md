---
title: "Tetris 手搓手册"
description: "从空自绘 QWidget 一行行搓出俄罗斯方块：4×4 包络形态表 + QPainter 逐格画棋盘 + QTimer 等级加速下落 + 旋转壁踢/碰撞/锁定 + 消行计分 + 副面板 Next 预览 + QSettings 最高分持久化。"
---

# Tetris 手搓手册

> **source**：成品答案在 `app/08-games/tetris/`（做完对照）· **related**：app 栏游戏类整机成品

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个俄罗斯方块，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **QPainter 自绘**：`paintEvent` 里 `fillRect`/`drawLine`/`drawRect` 逐格画棋盘网格 + 已堆方块 + 当前方块 + 投影 + 遮罩
- **QTimer 游戏循环**：`timeout` 信号驱动方块下落，`setInterval` 随等级递减实现加速
- **QKeyEvent 键盘路由**：`keyPressEvent` 接方向键，`Qt::StrongFocus` 让自绘控件拿到焦点
- **自定义控件数据建模**：把「棋盘」「方块」「旋转态」抽象成矩阵，`collides`/`lockPiece`/`clearFullLines` 纯算法
- **立体感渲染**：`QColor::lighter`/`darker` 画高光与阴影，让方块有体积
- **信号驱动副面板**：棋盘发 `statsChanged()`，主窗口刷 Next 预览 / 分数 / 等级——状态同步
- **QSettings 持久化**：把最高分存进注册表/ini，重开程序还在
- **QMainWindow 整机装配**：QHBoxLayout 棋盘 + 副面板、菜单 Game/Help、状态栏

## 1. 起点

先有个能跑的空自绘画布。最小 Qt Widgets 工程，主窗口里放一个自绘 QWidget：

```cpp
#include <QApplication>
#include <QMainWindow>
class Canvas : public QWidget {
  protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.fillRect(rect(), QColor(24, 24, 28));  // 黑底画布
    }
};
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QMainWindow w;
    w.resize(460, 600);
    w.setCentralWidget(new Canvas);
    w.show();
    return app.exec();
}
```

弹出黑底窗口 = 环境通了。QPainter 不熟先看 [QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md)，QMainWindow 不熟看 [QMainWindow 主窗口](../../../../../beginner/03-qtwidgets/55-qmainwindow-beginner.md)。

> 注意：工程链 `Qt6::Widgets` 即可（游戏自绘不用 Sql/Network）。`CMakeLists.txt` 的 `target_link_libraries` 里加 `Qt6::Core Qt6::Gui Qt6::Widgets`。

## 2. 任务清单

分 3 步（一文件/一阶段一步），每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 棋盘数据建模 + 7 形态方块 + QPainter 自绘渲染 + QTimer 自然下落 | [01](./01-board-and-paint.md) |
| 2 | 键盘控制（移/旋转/软降/硬降）+ 碰撞 + 壁踢 + 锁定 + 消行计分 + 等级加速 | [02](./02-control-lock-and-clear.md) |
| 3 | 副面板（Next 预览 + 分数面板）+ 菜单 + 暂停/重开 + QSettings 最高分持久化 | [03](./03-sidepanel-and-persist.md) |

成品对照：`app/08-games/tetris/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **真 SRS 踢墙**：现在用简化版 6 位置壁踢。提示：查 Super Rotation System 的 JLSTZ/I 两套踢墙表，按方块分流转置偏移。
- **7-bag 随机出块**：现在 `bounded(7)` 纯随机（可能连出同型）。提示：7 块洗牌成一组，一组出完再洗，保证每 7 块每种恰好一次。
- **Hold 槽**：加一个「暂存当前块、换出 Hold 槽里的块」功能。提示：副面板加第二个预览，按 C 交换。
- **音效**：锁定/消行/Tetris（4 连消）配音。提示：`QSoundEffect` 预加载 wav，事件触发播放。
- **软降/硬降分计 + T-spin 检测**：现在硬降 +2/格、软降 +1/格。提示：T-spin 检测 T 型旋转锁定时周围三角位占用，给额外分。
- **重绘优化**：现在每个 tick `update()` 全量重绘。提示：用脏区只刷变化的格子（`update(QRegion)`），高帧率场景省 CPU。
- **下一站**：app 栏的 image-viewer / serial-tool——换皮复用 QMainWindow + 自绘骨架，但引入图片查看 / QSerialPort。
