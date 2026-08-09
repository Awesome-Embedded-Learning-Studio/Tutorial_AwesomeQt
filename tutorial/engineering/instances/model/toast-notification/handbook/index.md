---
title: "ToastNotification 手搓手册"
description: "从空 main 一行行搓出 ToastNotification：三阶段打通无边框置顶窗口、windowOpacity 淡入淡出与定时自毁、单例堆叠管理。"
---

# ToastNotification 手搓手册

> **source**：成品答案在 `model/12-design-patterns/toast-notification/`（做完对照）· **related**：动画框架 · 定时器 · 对象树

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 ToastNotification，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **临时顶层窗口的标志位组合**：无边框 + 工具窗口 + 置顶 + 不抢焦点，四件套缺一不可
- **驱动 `windowOpacity` 做淡入淡出**：`QPropertyAnimation` 驱动 QWidget 自带属性，比 `QGraphicsOpacityEffect` 更适合顶层窗口
- **短生命周期对象的自我销毁**：`QTimer::singleShot` + `deleteLater` + `WA_DeleteOnClose` 闭环
- **单例模式 + 活动队列**：全局唯一管理器维护活动对象列表，做堆叠定位与回收重排
- **`destroyed` 信号驱动重排**：对象自毁时通知管理者更新布局

## 1. 起点

先有个能跑的空壳。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    w.resize(200, 80);
    w.show();
    return app.exec();
}
```

弹出空白窗 = 环境通了，往下走。Qt 环境不熟先看 [QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

## 2. 任务清单

分三阶段，每阶段：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| 阶段 | 目标 | 进 |
|---|---|---|
| 1 | 单条 Toast：无边框置顶 + 圆角自绘 + 不抢焦点 | [01](./01-frameless-window.md) |
| 2 | 淡入淡出 + 定时自毁（windowOpacity + QTimer + deleteLater） | [02](./02-fade-and-self-destruct.md) |
| 3 | 单例 ToastManager：多条堆叠 + 消失重排 | [03](./03-stacking-manager.md) |

成品对照：`model/12-design-patterns/toast-notification/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 5. 进阶挑战（可选）

搓完基础版想再深一层：

- **多角落共存**：本实现的堆叠队列是全局单一列表，简化为「同一时刻只跟一个角落」。挑战：改成 `QHash<ToastCorner, QList<Toast*>>`，让四个角落各自独立堆叠互不干扰。思考：`placeToast` 里「累计 y 偏移」那段按角落分桶会怎么变？
- **手动关闭**：加一个右上角小 × 按钮，点击立即 `fadeOut`。提示：`paintEvent` 里画一个 × 区域 + 重写 `mousePressEvent` 命中判定，或直接叠一个 `QToolButton`。
- **动画曲线**：淡入用 `OutCubic`（快进慢出，更自然）、淡出用 `InQuad`（慢进快出）。提示：`opacity_anim_->setEasingCurve(...)`。
- **下一站**：通知中心 / 通知队列——把「到点淡出」升级为「可暂停、可累积、可逐条关闭」的完整通知系统，引入状态机。
