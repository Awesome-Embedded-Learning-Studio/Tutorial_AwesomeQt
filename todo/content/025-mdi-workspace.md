---
id: "025"
title: "QtWidgets 入门：MDI 多文档界面与工作区"
category: content
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: []
estimated_effort: medium
---

## 目标

掌握 QMdiArea 和 QMdiSubWindow 的使用，学会构建 MDI (Multiple Document Interface)
多文档界面应用。理解 MDI 与 SDI 模式的区别，掌握子窗口的管理、排列与交互。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QMdiArea 与 QMdiSubWindow 的关系
- 子窗口创建与管理：addSubWindow, removeSubWindow
- 子窗口排列：cascadeSubWindows, tileSubWindows
- 子窗口模式：SubWindowView vs TabView
- 活动子窗口追踪：activeSubWindow, subWindowActivated 信号
- 子窗口的关闭策略
- 菜单与子窗口联动（窗口列表）
- QMdiArea 的背景设置
- 子窗口间数据共享与通信

踩坑重点：
1. 子窗口关闭后指针未置空导致悬空指针访问
2. TabView 模式下子窗口无法拖动，用户困惑
3. 大量子窗口未限制数量导致性能下降

练习项目：实现一个 MDI 文本编辑器，支持同时打开多个文本文件，
提供级联/平铺/标签切换视图模式，窗口菜单列出所有打开的文档。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/10-mdi-workspace-beginner.md
- examples/beginner/03-qtwidgets/10-mdi-workspace-beginner/

## 参考资料

- [QMdiArea Class Reference](https://doc.qt.io/qt-6/qmdiarea.html)
- [QMdiSubWindow Class Reference](https://doc.qt.io/qt-6/qmdisubwindow.html)
- [MDI Example](https://doc.qt.io/qt-6/qtwidgets-mainwindows-mdi-example.html)
