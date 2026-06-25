---
title: "卡住怎么办"
description: "按症状查：折线飞屏/NaN、空数据崩溃、Y 刻度数字糊折线、单点画飞、demo 编译失败——给方向指向成品 file:行号，不给完整答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/line-chart/`，对照着看。

## 折线画飞 / 点跑到屏幕外 / NaN

- Y 域退化时**有没有做 ±padding**？单点或常量序列会让 `y_max - y_min == 0`，`to_y` 分母为零得 NaN。判 `qFuzzyCompare(y_min, y_max)` 后给 `y_min -= 1; y_max += 1`。→ `src/line_chart.cpp:177-181`
- X 坐标单点**有没有单独写分支**？`i/(n-1)` 在 `n==1` 时分母为零。单点居中 `plot_left + plot_width/2`。→ `src/line_chart.cpp:233-234`
- `y_range` **是不是在 auto-scale 之外的地方被用到**了？auto-scale 必须在画线之前算完。
- 进阶排查：[QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md)、[坐标变换](../../../../../beginner/02-qtgui/02-coordinate-transform-beginner.md)

## 空数据时崩溃或表现诡异

- 空数据**有没有 early return**？画完网格/坐标轴后，若 `!has_data` 直接 return，别去建空 QPolygonF（`poly.first()/last()` 在空 polygon 上行为未定义）。→ `src/line_chart.cpp:225-227`
- `area` 填充**是不是只对 n>=2 才画**？单点没面积，硬画 `addPolygon` + `first()/last()` 可能出问题。→ `src/line_chart.cpp:239`
- 进阶排查：[容器](../../../../../beginner/01-qtbase/04-container-beginner.md)

## Y 轴刻度数字和折线糊在一起

- 数字**是不是用了固定起点 drawText**？左对齐时数字位数一变就往右溢进折线区。用 `QFontMetrics::horizontalAdvance` 量宽，起点设 `plot_left - text_w - 4` 右对齐到 Y 轴左侧。→ `src/line_chart.cpp:219-221`
- 垂直对齐**是不是没做 baseline 修正**？`drawText(QPointF(x, y + fm.ascent()/2), text)` 让数字大致垂直居中在网格线上。→ `src/line_chart.cpp:221`
- 进阶排查：[字体与文本渲染](../../../../../beginner/02-qtgui/04-font-text-beginner.md)

## 勾选框切不动外观 / 切了没反应

- `showGrid` / `showDots` / `showArea` 的 setter **末尾有没有 `update()`**？没 update 就不重绘。→ `src/line_chart.cpp:107`、`:121`、`:134`
- setter **有没有「值未变就 return」的守卫**？这步不是必须，但能省无谓重绘——注意别把 return 写在 update 后面。→ `src/line_chart.cpp:103-105`
- demo 里 `connect(checkbox, &QCheckBox::toggled, chart, &LineChart::setShowGrid)` 这种**函数指针语法**对得上吗？setter 签名是 `void setShowGrid(bool)`，和 `toggled(bool)` 一致。→ `demo/line_chart_window.cpp:95-97`
- 进阶排查：[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

## demo 编译失败（std::clamp / std::rand 找不到）

- 用了 `std::clamp` **有没有 `#include <algorithm>`**？用了 `std::rand` **有没有 `#include <cstdlib>`**？GCC 在 `-Wall` 下缺这俩头会直接编译失败。→ `demo/line_chart_window.cpp:9-10`
- 提醒：`qreal` 是 `double` 的 typedef，clamp 模板实参推导本身不一定报错，别指望编译器提醒你补头——STL 算法函数的 include 要显式写全。
- 进阶排查：[C++ 标准库容器与算法](../../../../../beginner/01-qtbase/04-container-beginner.md)

## moc 报错（Q_PROPERTY 不认识）

- 头文件**有没有 `Q_OBJECT`**？→ `include/line_chart.h:23`
- CMake **有没有开 AUTOMOC**（根 `widget/CMakeLists.txt` 的 `set(CMAKE_AUTOMOC ON)`）？
- Q_PROPERTY 的 READ/WRITE/NOTIFY 三件**签名对得上**吗？getter `const`、setter 参数类型、signal 参数类型要和声明一致。
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)

---

实在卡死成品就是答案——但先自己拼。对照 `widget/line-chart/` 的 `src/line_chart.cpp` 逐段看，按 [成品导览](../) 的「怎么读」顺序最省力。
