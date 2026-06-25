---
title: "RangeSlider 手搓手册"
description: "从空 main 一行行搓出 RangeSlider：3 步打通自绘控件骨架、值/像素映射 + 双手柄 hit-test、端点统一夹值约束。"
---

# RangeSlider 手搓手册

> **source**：成品答案在 `widget/range-slider/`（做完对照）· **related**：自绘控件递进链第 3 环（toggle-switch · status-led）· [自定义控件绘制入门](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 `RangeSlider`，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **自定义控件骨架**：继承 QWidget + 重写 `paintEvent` + 实现 `sizeHint` / `minimumSizeHint`
- **值/像素双向映射**：把逻辑值 `[minimum, maximum]` 和像素 x 互相换算，是所有滑块类控件的命脉
- **双值约束不变式**：`lower <= upper` 永远成立——setter 内 `std::clamp` + 端点统一夹值流程
- **双手柄 hit-test**：照搬 toggle-switch 的三事件 + 拖动阈值，扩成"离谁近抓谁"
- **Q_PROPERTY 全套**：4 个值属性 + 3 个配色属性全 READ/WRITE/NOTIFY

如果你已经搓过 toggle-switch，本手册会轻松很多——三事件 + 拖动阈值那一套原样复用，新增的核心是"双值约束"和"双手柄命中测试"。

## 1. 起点

先有个能跑的空壳。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    w.resize(200, 24);
    w.show();
    return app.exec();
}
```

弹出空白窗 = 环境通了，往下走。Qt 环境不熟先看 [QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

## 2. 任务清单

分 3 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 画轨道 + 双手柄 + 值/像素映射（静态可调） | [01 画轨道和双手柄](./01-paint-track-handles.md) |
| 2 | 双手柄拖动交互（hit-test + 阈值） | [02 拖动交互](./02-drag-handles.md) |
| 3 | 区间约束 + 端点统一夹值 | [03 约束与端点夹值](./03-constraints-range.md) |

成品对照：`widget/range-slider/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **强制两手柄最小间距**：把 `kHandleGap` 调成正值（值单位），让两手柄无法重合，体验更接近 Excel 的范围筛选器。思考：这跟 hit-test 的容差该怎么配合才不会"间距大但点不动"？
- **垂直方向**：把 `valueToX` / `xToValue` 换成 y 维度，事件改读 y。体会"映射函数抽象成一对正反函数后，换轴只改一行"的好处。
- **步进 / 吸附刻度**：拖动时把值 `round` 到整数倍 step。提示：在 `xToValue` 出口处吸附，而不是在 setter 里——这样程序化设值和拖动用同一套吸附。
- **下一站**：带动画的进度类控件（circle-progress）——本控件没要动画，是想让你专注"约束 + 映射"这两件事；进度控件会反过来，专注动画、约束退居幕后。
