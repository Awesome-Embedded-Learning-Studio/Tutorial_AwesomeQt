---
id: "013"
title: "QtGui 入门：字体与文本渲染"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["010"]
blocks: []
estimated_effort: medium
---

## 目标

掌握 Qt 的字体系统与文本渲染机制，包括 QFont 的配置、文本度量、
富文本处理、文本绘制选项。理解文本布局与换行的处理方式。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QFont：字体族、大小、粗细、斜体、下划线、删除线
- QFontMetrics / QFontMetricsF：文本度量（宽度、高度、 ascent/descent）
- QPainter::drawText() 的多种重载
- QTextOption：换行模式、对齐方式、文本方向
- QTextLayout：复杂文本布局
- QTextDocument：富文本渲染
- 文本裁剪与 ElideMode
- 多行文本绘制
- 字体回退机制

踩坑重点：
1. 使用固定像素值而非 QFontMetrics 计算文本尺寸，不同 DPI 下显示错位
2. 中文文本在未指定支持中文的字体族时显示为方块
3. drawText rect 重载中未考虑对齐导致文本位置偏差

练习项目：实现一个文本信息展示控件，支持自动换行、省略号裁剪、多字体混排。

## 涉及文件

- document/tutorials/beginner/02-qtgui/04-font-text-rendering-beginner.md
- examples/beginner/02-qtgui/04-font-text-rendering-beginner/

## 参考资料

- [QFont Class Reference](https://doc.qt.io/qt-6/qfont.html)
- [QFontMetrics Class Reference](https://doc.qt.io/qt-6/qfontmetrics.html)
- [QTextLayout Class Reference](https://doc.qt.io/qt-6/qtextlayout.html)
