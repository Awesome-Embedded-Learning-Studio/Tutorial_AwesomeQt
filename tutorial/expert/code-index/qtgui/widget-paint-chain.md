---
title: widget 绘制链——UpdateRequest 驱动与引擎真身
description: widget 重绘由 QEvent::UpdateRequest 驱动（syncBackingStore→QWidgetRepaintManager::sync→paintAndFlush 的 beginPaint/drawWidget/endPaint/flush 全链）、Paint 事件是 drawWidget 内部 sendPaintEvent 经 sendSpontaneousEvent 的中段同步补发、QWidget::paintEngine 空壳打警告返回 nullptr（引擎真身在 backingstore 的 QImage 上）、QBackingStore::paintDevice 转发平台（raster 缺省返回内部 m_image）、flush 并列双路（flushWithRhi 为真走 rhiFlush、为假走 store->flush，非失败回退）、xcb 终点 xcb_shm_put_image。
---

# widget 绘制链——UpdateRequest 驱动与引擎真身

> 本索引收录 Qt 6.9.1 源码中 widget 绘制路径相关的已验证证据。对应专家篇《01 QPainter 后端架构》widget 绘制链一节（本篇立论支柱，B 逐环复核闭合）。

## 驱动链

源码文件：`qtbase/src/widgets/kernel/qwidget.cpp` / `qwidgetrepaintmanager.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Paint 事件只调 paintEvent | qwidget.cpp:9144-9149 | `case QEvent::Paint: paintEvent(...)` | 真正拉起整轮的是 UpdateRequest。 |
| UpdateRequest→syncBackingStore | qwidget.cpp:9311-9313 | `case QEvent::UpdateRequest: d->syncBackingStore()` | 转发 repaintManager（1772-1779）。 |
| sync 转入 paintAndFlush | qwidgetrepaintmanager.cpp:356-370（sync 638-660 见正文） | markDirty 分流 | sync 本体 638-660，paintAndFlush 头 703。 |
| paintAndFlush 主体 | qwidgetrepaintmanager.cpp:883-912 | beginPaint(873)→drawWidget(store->paintDevice())→endPaint+flush() | 绘制发生在后备存储的设备上。 |
| Paint 事件中段补发 | qwidget.cpp:5667-5671 | sendPaintEvent 经 sendSpontaneousEvent 同步直投 | 调用点在 drawWidget（5554-5561 取 pdev 引擎）。 |

## 引擎真身与 flush

源码文件：`qtbase/src/widgets/kernel/qwidget.cpp` / `gui/painting/qbackingstore.cpp` / `qrasterbackingstore.cpp` / `plugins/platforms/xcb/qxcbbackingstore.cpp` / `qwidgetrepaintmanager.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QWidget::paintEngine 空壳 | qwidget.cpp:12657-12660 | `qWarning("QWidget::paintEngine: Should no longer be called")` ... return nullptr（12679） | Qt5 的 QWidgetPaintEngine 类全 qtbase grep 零命中。 |
| paintDevice 转发平台 | qbackingstore.cpp:157-165 | handle()->paintDevice()，highDpi 分支 | raster 缺省返回 &m_image（qrasterbackingstore.cpp:37-40）。 |
| flush 并列双路 | qwidgetrepaintmanager.cpp:1042-1086 | flushWithRhi 为真走 rhiFlush（1068-1073）、为假走 store->flush（1085-1086） | rhiFlush 失败走 graphicsDeviceReportedLost 加 update()（1077-1086），不回退。 |
| xcb 终点 | qxcbbackingstore.cpp:569-573 + 755-761 | shmPutImage→xcb_shm_put_image | 共享内存推 X drawable，widget 从不直接画窗口。 |
