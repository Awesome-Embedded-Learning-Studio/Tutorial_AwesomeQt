---
title: 平台钩子与 GL 引擎——三家工厂与当代形态
description: QPlatformIntegration 三个工厂（createPlatformPixmap 缺省 QRasterPlatformPixmap、createImagePaintEngine 缺省 nullptr 使 QImage 回落 raster、createPlatformBackingStore 纯虚由插件实现 xcb 三选一 Windows GDI 一选）、QOpenGL2PaintEngineEx 住 QtOpenGL 模块非插件（QOpenGLPaintDevice 按线程局部缓存创建、QOpenGLPaintEngine 类定义已死仅 qpainter.h 残留 friend 声明）、QPaintEngine::Type 枚举 17 常规成员加 User=50 到 MaxUser=100（User 区 qtbase 零使用，SVG 是原生枚举非 User 区）。
---

# 平台钩子与 GL 引擎——三家工厂与当代形态

> 本索引收录 Qt 6.9.1 源码中引擎平台选择相关的已验证证据。对应专家篇《01 QPainter 后端架构》平台钩子一节。

## 三家工厂

源码文件：`qtbase/src/gui/kernel/qplatformintegration.cpp` / `qplatformintegration.h` / `plugins/platforms/xcb/qxcbintegration.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| createPlatformPixmap 缺省 raster | :265-268 | `return new QRasterPlatformPixmap(type);` | QPixmap 默认走 raster 的原因。 |
| createImagePaintEngine 缺省空 | :313-317 | `return nullptr;` | QImage 因此回落 new QRasterPaintEngine。 |
| createPlatformBackingStore 纯虚 | qplatformintegration.h:113 | `virtual QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const = 0;` | xcb 三选一（qxcbintegration.cpp:273-287）、Windows GDI 给 QWindowsBackingStore。 |

## GL 引擎与 Type 枚举

源码文件：`qtbase/src/opengl/qopenglpaintengine_p.h` / `qopenglpaintdevice.cpp` / `gui/painting/qpainter.h` / `gui/painting/qpaintengine.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QOpenGL2PaintEngineEx | qopenglpaintengine_p.h:73 | `class Q_OPENGL_EXPORT QOpenGL2PaintEngineEx : public QPaintEngineEx` | 住 src/opengl 的 QtOpenGL 模块，非插件；QOpenGLPaintEngine 无类定义仅 qpainter.h:431 残留 friend 声明。 |
| 每线程引擎缓存 | qopenglpaintdevice.cpp:146-156 | QOpenGLEngineThreadStorage 里 new QOpenGL2PaintEngineEx | 用户是 QOpenGLWidget 与 QOpenGLWindow。 |
| Type 枚举 17 常规成员 | qpaintengine.h:157-176 | X11 到 Direct2D 十七个加 User=50/MaxUser=100 | User 区 qtbase 零使用；QSvgEngine::type() 返回原生 SVG 枚举（qtsvg/src/svg/qsvggenerator.cpp:171，跨模块单点验证）。 |
