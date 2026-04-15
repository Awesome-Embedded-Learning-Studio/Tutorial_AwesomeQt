---
id: "020"
title: "QtWidgets 入门：自定义绘制控件"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["010", "016"]
blocks: []
estimated_effort: medium
---

## 目标

掌握基于 QWidget 的自定义绘制控件开发方法，结合 QPainter 与事件处理，
构建具备交互能力的自定义控件。理解控件开发的完整流程：
设计、绘制、事件处理、属性系统、样式集成。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QWidget 子类化的标准模式
- paintEvent() 重写与双缓冲
- 鼠标事件处理：mousePressEvent, mouseMoveEvent, mouseReleaseEvent
- 键盘事件处理：keyPressEvent, keyReleaseEvent
- Q_PROPERTY 与自定义属性声明
- sizeHint() 与 minimumSizeHint() 的合理实现
- update() vs repaint() 的区别
- 控件状态管理
- 信号与槽的合理设计
- QSS 与自定义控件的集成 (paintEvent 中读取 QStyle)

踩坑重点：
1. 不使用双缓冲导致频繁重绘时闪烁严重
2. 忘记设置 setMouseTracking(true) 导致 mouseMoveEvent 不触发
3. 析构函数中未断开信号连接导致野指针回调

练习项目：实现一个自定义的圆形进度条控件，支持动画过渡、渐变色绘制、鼠标交互调节值。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/05-custom-painted-widgets-beginner.md
- examples/beginner/03-qtwidgets/05-custom-painted-widgets-beginner/

## 参考资料

- [QWidget Class Reference](https://doc.qt.io/qt-6/qwidget.html)
- [Custom Widget Example](https://doc.qt.io/qt-6/qtwidgets-widgets-customwidget-example.html)
- [Analog Clock Example](https://doc.qt.io/qt-6/qtwidgets-widgets-analogclock-example.html)
