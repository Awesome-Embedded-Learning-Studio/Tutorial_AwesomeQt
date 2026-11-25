---
title: begin 与引擎归属——设备造引擎画笔接管
description: QPainter::begin 七分支失败清单（单 painter/已激活/引擎空/null 图/Indexed8 与 CMYK8888/engine begin 假）、QPaintDevice::paintEngine 纯虚由设备创建（QImage 惰性缓存、QPixmap 两级转发到内部 QImage）、QPaintEngineDestructor 仅 autoDestruct 真才 delete 且 setAutoDestruct 全树零调用构造初始化 false（所有权稳定归设备）、ushort painters 计数器与 sharedPainter 例外通道、end 收尾链。
---

# begin 与引擎归属——设备造引擎画笔接管

> 本索引收录 Qt 6.9.1 源码中 QPainter 生命周期与引擎所有权相关的已验证证据。对应专家篇《01 QPainter 后端架构》begin 一节。

## begin 与失败分支

源码文件：`qtbase/src/gui/painting/qpainter.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 单 painter 与已激活两道拒绝 | :1679-1689 | `if (pd->painters > 0) { qWarning("...one painter at a time."); return false; } if (d_ptr->engine) { ... }` | 七分支中的前两道。 |
| 引擎为空分支 | :1714-1719 | `d->engine.reset(pd->paintEngine()); if (!d->engine) { qWarning("...returned engine == 0, type: %d"...)` | 设备给不出引擎即失败。 |
| null 图与格式拒绝 | :1766-1774 | null pixmap、null image、Format_Indexed8 与 Format_CMYK8888 | 共三分支。 |
| begin 尾部计数与 metric | :1812-1822 | `++d->device->painters`（1829-1833）与 metric(PdmWidth/Height) 初始化、coordinateOffset 并入 redirectionMatrix | 计数与窗口视口默认值。 |
| sharedPainter 例外通道 | :1691-1692 | attachPainterPrivate 提前 return true 不走 ++ | 共享 d_ptr 在计数器眼里仍是一个 painter（通道本体 242-252）。 |
| end 收尾 | :1869-1884 | engine->end、updateState(nullptr)、减计数归零断引擎、警告未配平 save 栈、reset 引擎指针 | 析构只调 end（1474-1482）。 |

## 引擎归属

源码文件：`qtbase/src/gui/painting/qpaintdevice.h` / `qpainter_p.h` / `gui/image/qimage.cpp` / `gui/image/qpixmap.cpp` + `qpixmap_raster.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| paintEngine 纯虚 | qpaintdevice.h:41 | `virtual QPaintEngine *paintEngine() const = 0;` | 创建责任在设备子类。 |
| 删除器仅 autoDestruct 真 delete | qpainter_p.h:233-240 | `if (pe && pe->autoDestruct()) delete pe;` | setAutoDestruct 是 private 且 qtbase 全树零调用、构造初始化 false（qpaintengine.cpp:666/681）——引擎恒不被 painter 删除。 |
| ushort painters 计数 | qpaintdevice.h:66 | `ushort painters; // refcount` | 单画笔协定的本体。 |
| QImage 惰性创建缓存 | qimage.cpp:4284-4299 | 先问平台 createImagePaintEngine、空则 new QRasterPaintEngine，缓存于 QImageData | 同图重复 begin 同一引擎。 |
| QPixmap 两级转发 | qpixmap.cpp:1339-1342 + qpixmap_raster.cpp:219-222 | `data->paintEngine()` → `return image.paintEngine();` | raster 平台 QPixmap 绘制本质是内部 QImage。 |
