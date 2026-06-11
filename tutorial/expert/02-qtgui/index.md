---
title: "02 · QtGui 绘图与图像（专家）"
description: "深入绘图系统源码：QPainter 渲染后端与 QPaintEngine、QTransform 矩阵运算优化、QImage 像素格式与内存布局、字体引擎 HarfBuzz 集成、OpenGL 上下文管理、拖放平台 DnD 协议封装。共 6 篇。"
---

# 02 · QtGui 绘图与图像（专家）

> 规划中（6 篇），敬请期待...

## 章节规划

- **01 QPainter 源码** — `QPaintEngine` 抽象层（Raster/OpenGL/PDF）、`updateState()` 状态机、`QRasterPaintEngine` 扫描线填充
- **02 QTransform 矩阵运算** — 3x3 矩阵类型分级、矩阵乘法优化、`map()` 几何类型分发、`squaredNorm()` 碰撞检测
- **03 QImage 像素格式与内存** — `Format` 枚举字节排列、`QImageData` 引用计数、`convertTo()` lookup table 优化、高 DPI devicePixelRatio
- **04 字体引擎源码** — `QFontEngine` 平台实现（FreeType/DirectWrite/CoreText）、HarfBuzz 整形、`QFontCache` 缓存、字体回退链
- **05 OpenGL 上下文管理** — `QPlatformOpenGLContext` 平台抽象、`makeCurrent()` 线程约束、`QOpenGLSharedResourceGuard` 资源管理
- **06 拖放系统源码** — `QDragManager` 状态机、X11 XDND / Windows OLE IDropSource、`QMimeData` 延迟数据提供
