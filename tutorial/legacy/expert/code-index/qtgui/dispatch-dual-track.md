---
title: 分发与状态双轨——即时通知与脏标志
description: drawLines 四段式（extended 优先、updateState、emulationSpecifier 补偿、落虚函数）、QPaintEngine 纯虚恰 5 个默认实现 14 个、两级回退（drawEllipse 拆 path 或凸多边形、drawRects 双路）与 drawPath 默认空实现、fillPath 的换笔手法、状态同步双轨（extended 即时 penChanged/transformChanged 与非 extended 脏标志 updateStateImpl）、setPen 相等短路、四矩阵 painter 侧合成、viewTransform 纯 painter 概念、save/restore 引擎深参与、drawTextItem 默认退化成路径。
---

# 分发与状态双轨——即时通知与脏标志

> 本索引收录 Qt 6.9.1 源码中绘制分发与状态同步相关的已验证证据。对应专家篇《01 QPainter 后端架构》分发与状态一节。

## 分发链与回退

源码文件：`qtbase/src/gui/painting/qpainter.cpp` / `qpaintengine.cpp` / `qpaintengine.h` / `qpaintengineex.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| drawLines 四段式 | qpainter.cpp:4319-4346 | extended 判定、updateState、emulationSpecifier 手工平移补偿、落 engine 虚函数 | 非 extended 老轨的完整样板。 |
| 纯虚 5 个默认 14 个 | qpaintengine.h:117-143 | begin/end/updateState/drawPixmap/type 纯虚；draw 系 122-143 带默认 | drawPolygon 至少实现一个版本（文档注记 qpaintengine.cpp:293-296）。 |
| drawEllipse 双回退 | qpaintengine.cpp:471-481 | PainterPaths 有则 drawPath 否则 toFillPolygon 加 ConvexMode | 基类拆低阶图元。 |
| drawPath 默认实现是空的 | qpaintengine.cpp:707-712 | 未实现却声明特性时 qWarning | fill+stroke 拆解在 painter 侧：fillPath 的 setPen(NoPen) 手法（qpainter.cpp:3117-3127）。 |
| extended 收逻辑坐标 | qpaintengineex.cpp:717-731 | drawRects 原样构造 QVectorPath 调 draw(vp) | 变换由引擎内部做。 |

## 状态双轨

源码文件：`qtbase/src/gui/painting/qpainter.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 脏标志同步 | :860-890 | engine->state=newState、updateState、clearDirty(AllDirty) | 改 N 次状态绘制时同步一次。 |
| setPen 相等短路 | :3653-3671 | `if (d->state->pen == pen) return;` 变更仅标脏；extended 走 penChanged | 惰性与即时两轨的分岔点。 |
| 四矩阵合成 | :626-643 | worldMatrix、viewTransform、redirectionMatrix、hidpiScaleTransform 乘成单个 state->matrix | window/viewport 是纯 painter 概念（viewTransform 207-216，八整数存 QPainterState）。 |
| save/restore 深参与 | :1558-1570 | extended 走 createState/setState、非 extended 拷贝构造压栈、engine->state 跟栈走 | changeFlags 并入 dirtyFlags 在 updateStateImpl（873-878）非 restore 体内。 |
| 文本退化 | qpaintengine.cpp:743-753 | ARGB 字形逐个位图 drawImage、普通字形 addOutlineToPath 转路径 fillPath | 引擎不实现文本时文本变矢量路径。 |
