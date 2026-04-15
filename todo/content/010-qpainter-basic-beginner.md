---
id: "010"
title: "QtGui 入门：QPainter 绘图基础"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: ["011", "012", "013", "014", "015", "016"]
estimated_effort: medium
---

## 目标

掌握 QPainter 的基本使用方法，包括画笔 (QPen)、画刷 (QBrush) 的配置，
以及基本图形 (线段、矩形、椭圆、多边形、弧线) 的绘制。
理解 QWidget::paintEvent 的工作机制，建立自定义绘制的核心能力。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QPainter 的创建与生命周期管理
- QPen (颜色、宽度、线型、端点样式、连接样式)
- QBrush (纯色、渐变、纹理填充)
- 基本图形绘制：drawLine, drawRect, drawEllipse, drawPolygon, drawArc, drawChord, drawPie
- 渐变填充：QLinearGradient, QRadialGradient, QConicalGradient
- QPainterPath 路径绘制
- 抗锯齿与渲染提示 (QPainter::RenderHint)
- 坐标系统基础介绍

踩坑重点：
1. 在 paintEvent 外创建 QPainter 导致崩溃
2. 未设置 RenderHint 导致图形锯齿严重
3. 忘记 save/restore 状态导致样式污染

练习项目：绘制一个包含多种基本图形的简单仪表盘面板。

## 涉及文件

- document/tutorials/beginner/02-qtgui/01-qpainter-basic-beginner.md
- examples/beginner/02-qtgui/01-qpainter-basic-beginner/

## 参考资料

- [QPainter Class Reference](https://doc.qt.io/qt-6/qpainter.html)
- [QPen Class Reference](https://doc.qt.io/qt-6/qpen.html)
- [QBrush Class Reference](https://doc.qt.io/qt-6/qbrush.html)
- [Paint System Overview](https://doc.qt.io/qt-6/paintsystem.html)
