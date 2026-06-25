---
title: "Step 5-6：配色/线宽 Q_PROPERTY + 几何 clamp 兜底"
description: "把 strokeWidth/progressColor/ringColor/showText 做成 Q_PROPERTY，半径几何 std::max(1.0,...) clamp 兜底，防控件压极小时 drawArc 行为未定义。"
---

# Step 5-6：配色/线宽 Q_PROPERTY + 几何 clamp 兜底

← [手册首页](./index.md) · 上一步 [Step 3-4 平滑过渡](./02-smooth-transition.md) →

## Step 5：配色 / 线宽 / 文字开关做成 Q_PROPERTY

### 目标

把 `strokeWidth`、`progressColor`、`ringColor`、`showText` 四个都升级成 Q_PROPERTY，外部能直接 setter 改、能被 Designer / 动画驱动。这样换主题色、调粗细、关文字都不用动控件源码。

### 提示

- 每个属性一套 READ / WRITE / NOTIFY 三件，setter 里：值变了才动 → 改成员 → `update()` → emit
- `setStrokeWidth` 里夹一下 `std::max(1, width)`，线宽不能 <=0
- `setShowText(bool)` 关文字后，paintEvent 里 `if (show_text_)` 才画文字
- 颜色用 `QColor`，Q_PROPERTY 直接支持，不用额外注册
- setter 里先判 `if (old == new) return;` 去重，避免无变化时白重绘 + 乱发信号

### 检查点

`setProgressColor(绿)` 弧变绿、`setStrokeWidth(18)` 弧变粗、`setShowText(false)` 中间文字消失 = 四个 Q_PROPERTY 都通。demo 的 Variants 区就是靠这几个 setter 设出来的。

### 对照答案

- 四个 Q_PROPERTY 声明：`include/circle_progress.h:31-34`
- setStrokeWidth / setProgressColor / setRingColor / setShowText：`src/circle_progress.cpp`（搜对应函数名）

---

## Step 6：几何 clamp 兜底

### 目标

控件被布局压到极小、或线宽设极大时，半径 `min(w,h)/2 - stroke/2 - 2` 会算成负数或 0。给所有几何尺寸加 `std::max(1.0, ...)` 兜底，保证 drawArc 永远拿到合法矩形。

### 提示

- 半径算出来后包一层 `std::max(1.0, r)`
- `side = std::max(1, std::min(width(), height()))`——宽高本身也可能极小
- 线宽 `std::max(1.0, stroke)`，别让线宽比直径还大
- 文字字号也兜底 `std::max(8.0, side*0.16)`，否则极小控件上字号算成 0 看不见
- 这是自绘控件的标配收尾：Qt 的 drawArc / drawEllipse 对负宽高矩形行为未定义，不 clamp 极端尺寸下会崩或乱画

### 检查点

把控件拖到 10×10、线宽拉到 30，弧不崩、不报错、不乱画线 = clamp 兜底到位。

### 对照答案

- side / 半径 clamp：`src/circle_progress.cpp:165`
- 线宽 clamp：`src/circle_progress.cpp:166`
- 字号 clamp：`src/circle_progress.cpp:199` 附近

---

到这里 CircleProgress 就搓齐了。回头看：drawArc 画弧 + value/progress 解耦 + QPropertyAnimation 接力 + Q_PROPERTY 收尾 + 几何 clamp——这就是「一个可被动画驱动的自绘控件」的完整范式。下一站 [speed-meter](../../speed-meter/) 会把这套骨架接到指针旋转上，再加刻度，难度又上一档。卡住了翻 [卡住怎么办](./troubleshooting.md)。
