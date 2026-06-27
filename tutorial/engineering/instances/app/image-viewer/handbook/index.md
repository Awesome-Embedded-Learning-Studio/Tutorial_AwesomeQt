---
title: "Image Viewer 手搓手册"
description: "从空 QMainWindow 一行行搓出图片查看器：自定义画布 paintEvent、QTransform 缩放旋转、QScrollArea 滚动、同目录翻页、幻灯片全屏。"
---

# Image Viewer 手搓手册

> **source**：成品答案在 `app/05-image-tools/image-viewer/`（做完对照）· **related**：app 栏整机应用范式第 1 件

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个图片查看器，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **自定义 QWidget + paintEvent**：继承 QWidget、重写 paintEvent、用 QPainter 画 QImage
- **QTransform 坐标变换**：一个矩阵合成「平移 → 缩放 → 旋转」，理解右乘累积的顺序
- **QScrollArea + sizeHint**：widgetResizable 策略 + 自定义部件尺寸驱动滚动范围
- **事件处理**：wheelEvent 滚轮缩放、keyPressEvent 键盘翻页
- **QMainWindow 整机装配**：菜单栏 / 工具栏 / 状态栏 / QAction 复用
- **QTimer**：幻灯片定时翻页 + 全屏状态管理

## 1. 起点

先有个能跑的空主窗口。最小 Qt Widgets 工程，main 里弹个 QMainWindow：

```cpp
#include <QApplication>
#include <QMainWindow>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QMainWindow w;
    w.resize(800, 600);
    w.show();
    return app.exec();
}
```

弹出空白主窗口 = 环境通了。QMainWindow 不熟先看 [QMainWindow 主窗口](../../../../../beginner/03-qtwidgets/55-qmainwindow-beginner.md)。

## 2. 任务清单

分 5 步（归到 3 个阶段文件），每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 自定义 ImageView + paintEvent 显示一张图 | [01](./01-load-and-display.md) |
| 2 | QTransform 加缩放 + 旋转 | [02](./02-zoom-and-rotate.md) |
| 3 | zoomToFit + 滚轮缩放 + 状态栏百分比 | [02](./02-zoom-and-rotate.md) |
| 4 | 同目录翻页（循环、跳坏图） | [03](./03-navigate-and-slideshow.md) |
| 5 | 幻灯片全屏 + 窗口状态恢复 | [03](./03-navigate-and-slideshow.md) |

成品对照：`app/05-image-tools/image-viewer/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **滚轮以鼠标位置为锚缩放**（zoom toward cursor）：现在缩放以画布中心为锚，缩放后鼠标下的点会跑偏。提示：缩放前记鼠标对应的图像坐标，缩放后调整 QScrollArea 的 scrollbar value 把该点拉回鼠标下。
- **大图预缩放缓存**：paintEvent 每次对原始大图做 QTransform 变换，高倍缩放会卡。提示：缩放比变化时预先 `QImage::scaled` 出一档缓存图，paintEvent 画缓存图。
- **Ctrl+滚轮缩放、裸滚轮滚动**：现在滚轮全用于缩放，放大后滚不动画面。提示：wheelEvent 里判断 `event->modifiers() & Qt::ControlModifier`，是 Ctrl 才缩放并 accept，否则不 accept 交给 QScrollArea 滚动。
- **拖拽平移**：放大后按住鼠标拖动平移画面。提示：mousePressEvent 记起点 + scrollbar value，mouseMoveEvent 算偏移调 scrollbar。
- **下一站**：app 栏的 sqlite-browser / serial-tool——换皮复用 QMainWindow 整机骨架，但引入 QSqlModel / QSerialPort。
