---
title: "Step 1：自定义画布 + 显示一张图"
description: "继承 QWidget 重写 paintEvent，用 QPainter 把 QImage 居中画出来，QImageReader 加载（含 EXIF 自动校正）。"
---

# Step 1：自定义画布 + 显示一张图

← [手册首页](./index.md) · 下一步 [Step 2-3 缩放与旋转](./02-zoom-and-rotate.md) →

## Step 1：自定义 ImageView + paintEvent 显示一张图

### 目标

窗口中央出现一块深色画布，打开一张图后**居中显示**，缩放窗口图不变形（先不缩放/旋转，下一步加）。

### 提示

- 新建 `ImageView` 继承 `QWidget`，加 `Q_OBJECT`（后面要发信号）
- 成员：`QImage image_`；`loadImage(const QString&)` 用 `QImageReader::read` 加载，顺手 `setAutoTransform(true)` 自动校正手机拍照的 EXIF 方向
- 重写 `protected void paintEvent(QPaintEvent*)`：`QPainter p(this)`、先 `fillRect(rect(), 深色)` 填背景、`drawImage` 把图画在画布中心
- 设 `setAttribute(Qt::WA_OpaquePaintEvent)`——paintEvent 自己填满背景，省一次系统擦除
- 加载失败要能反馈：`QImageReader::errorString()` + emit 一个 `loadFailed` 信号
- 主窗口里用 `QScrollArea` 包住 ImageView（`setWidget` 会接管所有权），`setCentralWidget(scrollArea)`

### 检查点

打开一张图，**居中显示在深色画布上**，缩放窗口图不动 = 显示链路通了。

> QPainter 不熟？先读 [QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md) 和 [自定义绘制 Widget 基础](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)。
> QImage / QImageReader 看 [图像与像素图](../../../../../beginner/02-qtgui/03-image-pixmap-beginner.md)。

### 对照答案

- paintEvent 填背景 + drawImage：`demo/image_view.cpp:117`
- QImageReader 加载 + EXIF：`demo/image_view.cpp:25-27`
- QScrollArea 包画布：`demo/image_viewer_window.cpp:137-148`

---

下一步是重头戏：[Step 2-3 用 QTransform 加缩放和旋转](./02-zoom-and-rotate.md)。
