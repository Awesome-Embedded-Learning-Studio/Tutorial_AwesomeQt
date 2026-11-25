---
title: raster 引擎——双路光栅化与 span 生产线
description: QRasterPaintEngine 经 QPaintEngineEx 孙辈继承且 type 是虚函数、begin 只管状态（Pixmap 统一成 QImage）而字节绑定在构造期 init（QRasterBuffer::prepare 取 image->bits）、双路光栅化同产 QT_FT_Span（非抗锯齿走自家 QRasterizer、抗锯齿走 FreeType 灰度）、QSpanData 按源类型配回调（blendColor/blendGradient/blendTexture 加 clip 包装）、合成模式函数表 functionForModeSolid 按 mode 索引且 static_assert 卡 NCompositionModes、抗锯齿开关经 renderHintsChanged 写 flags.antialiased。
---

# raster 引擎——双路光栅化与 span 生产线

> 本索引收录 Qt 6.9.1 源码中软件光栅引擎相关的已验证证据。对应专家篇《01 QPainter 后端架构》raster 引擎一节。

## 引擎骨架

源码文件：`qtbase/src/gui/painting/qpaintengine_raster_p.h` / `qpaintengine_raster.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 孙辈继承与 type 虚函数 | qpaintengine_raster_p.h:94 + 184 | `class QRasterPaintEngine : public QPaintEngineEx` + `Type type() const override { return Raster; }` | 不是 QPaintEngine 直系。 |
| begin 统一设备 | qpaintengine_raster.cpp:385-419 | Pixmap 的 RasterClass/BlitterClass 换 pd->buffer()、systemStateChanged、penData/brushData init | begin 只管状态。 |
| 字节绑定在构造期 | qpaintengine_raster.cpp:311-320 + 3747-3753 | init() 内构造灰度光栅（312 起）与 QRasterBuffer::prepare 取 image->bits()（3749） | 绑定不在 begin。 |

## 光栅化与回调

源码文件：`qtbase/src/gui/painting/qpaintengine_raster.cpp` / `qrasterizer.cpp` / `qdrawhelper.cpp` / `qcompositionfunctions.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| fillPolygon 转 outline | qpaintengine_raster.cpp:1866-1874 | QOutlineMapper::convertPath 转 QT_FT_Outline、取 ProcessSpans 混合函数 | outlineMapper（非下划线）。 |
| 双路分流 | :3539-3551 | 非抗锯齿 initializeRasterizer 走 QRasterizer::rasterize、抗锯齿走灰度光栅重载 | 两路同吐 QT_FT_Span。 |
| QRasterizer 产 span | qrasterizer.cpp:1166-1175 | QSpanBuffer（攒满 SPAN_BUFFER_SIZE 回调 m_blend） | 下潜止于此层。 |
| QSpanData 配回调 | qpaintengine_raster.cpp:4601-4635 | Solid→qDrawHelper[format].blendColor、渐变→qBlendGradient、纹理→qBlendTexture，clip 包装 qt_span_fill_clipRect/clipped | 笔刷数据包按源类型配混合函数。 |
| 合成模式函数表 | qcompositionfunctions.cpp:4212-4251 + 4253 | qt_functionForModeSolid_C[] 按 NCompositionModes 摆位，static_assert 卡尺寸 | 索引在 qdrawhelper.cpp:3901-3902（functionForModeSolid[op.mode] 与 functionForMode[op.mode]）。 |
| 抗锯齿开关 | qpaintengine_raster.cpp:786-798（写点）+ 3509 | renderHintsChanged 按 Antialiasing 位写 flags.antialiased、initializeRasterizer 传给 QRasterizer::setAntialiased | 双路分流的源头。 |
