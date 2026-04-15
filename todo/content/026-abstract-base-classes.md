---
id: "026"
title: "QtWidgets 控件速查：抽象基类 (QWidget/QFrame/QAbstractButton)"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: []
estimated_effort: medium
---

## 目标

深入理解 QtWidgets 控件体系的抽象基类设计，掌握 QWidget、QFrame、QAbstractButton
的核心 API 与继承关系。理解这些基类提供的通用能力，为后续学习具体控件打下基础。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QWidget：所有控件的根基
  - 窗口标志 (Qt::WindowFlags)
  - 属性系统：enabled, visible, geometry, sizePolicy, focusPolicy
  - 核心事件：show/hide, resize, move, close
  - 信号：customContextMenuRequested
  - 样式属性与 QStyle
- QFrame：带边框的控件基类
  - frameShape：NoFrame, Box, Panel, StyledPanel, HLine, VLine
  - frameShadow：Plain, Raised, Sunken
  - lineWidth, midLineWidth, frameRect
- QAbstractButton：按钮控件基类
  - 核心 API：setText, setIcon, setCheckable, setChecked
  - 核心信号：clicked, pressed, released, toggled
  - autoExclusive, autoRepeat
  - 虚函数：paintEvent, hitButton, checkStateSet

踩坑重点：
1. QWidget::setFixedSize 与 QSizePolicy 冲突导致布局异常
2. QFrame 边框在 QSS 中被覆盖导致 frameShape 不生效
3. QAbstractButton::clicked 信号在 disabled 状态下仍可通过代码触发

练习项目：实现一个继承自 QFrame 的自定义基类控件，支持多种边框风格切换，
演示 QWidget 属性系统在实际开发中的使用模式。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/controls/01-abstract-base-classes.md
- examples/beginner/03-qtwidgets/controls/01-abstract-base-classes/

## 参考资料

- [QWidget Class Reference](https://doc.qt.io/qt-6/qwidget.html)
- [QFrame Class Reference](https://doc.qt.io/qt-6/qframe.html)
- [QAbstractButton Class Reference](https://doc.qt.io/qt-6/qabstractbutton.html)
