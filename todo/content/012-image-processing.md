---
id: "012"
title: "QtGui 入门：图像处理与 QPixmap/QImage"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["010"]
blocks: []
estimated_effort: medium
---

## 目标

理解 Qt 图像类的层次结构，掌握 QPixmap、QImage、QBitmap、QPicture 的区别与使用场景。
学会图像的加载、保存、缩放、旋转、像素级操作，
以及图像与 QPainter 的配合使用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QImage vs QPixmap vs QBitmap vs QPicture 各类的适用场景
- QImage：像素级读写 (setPixelColor / pixelColor)、格式转换、镜像
- QPixmap：屏幕显示优化、缓存机制
- 图像加载与保存：支持格式 (JPEG, PNG, BMP, GIF 等)
- 图像缩放：Qt::TransformationMode (FastTransformation vs SmoothTransformation)
- QPainter 在 QImage 上绘制 (双缓冲技术)
- QImage::Format 枚举与格式转换
- QBuffer 内存中图像操作
- 图像混合模式 (QPainter::CompositionMode)

踩坑重点：
1. 在非 GUI 线程使用 QPixmap 导致崩溃（QPixmap 依赖 GUI 线程）
2. 大图像使用 SmoothTransformation 导致严重性能问题
3. QImage 格式不匹配导致颜色显示异常

练习项目：实现一个简易图片查看器，支持缩放、旋转、灰度转换功能。

## 涉及文件

- document/tutorials/beginner/02-qtgui/03-image-processing-beginner.md
- examples/beginner/02-qtgui/03-image-processing-beginner/

## 参考资料

- [QImage Class Reference](https://doc.qt.io/qt-6/qimage.html)
- [QPixmap Class Reference](https://doc.qt.io/qt-6/qpixmap.html)
- [Image Processing with Qt](https://doc.qt.io/qt-6/qtimageprocessing-index.html)
