---
id: "022"
title: "QtWidgets 入门：主窗口框架 (菜单/工具栏/状态栏/Dock)"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: []
estimated_effort: large
---

## 目标

掌握 QMainWindow 框架的完整使用，包括菜单栏 (QMenuBar)、工具栏 (QToolBar)、
状态栏 (QStatusBar)、停靠窗口 (QDockWidget)、中心部件的设置与管理。
学会构建功能完善的桌面应用主窗口。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QMainWindow 结构：menuBar, toolBars, statusBar, dockWidgets, centralWidget
- QMenuBar 与 QMenu：菜单创建、子菜单、分隔符、快捷键
- QAction：动作对象与信号槽连接、图标、快捷键、状态提示
- QToolBar：工具栏创建、浮动、可移动、合并、动作分组
- QStatusBar：临时消息、永久部件、进度指示
- QDockWidget：可停靠面板、嵌套、制表
- 中心部件：单文档 vs 多文档
- QMenuBar/QAction 的 UI 文件设计
- 窗口状态保存与恢复 (saveState/restoreState)

踩坑重点：
1. QAction 未设置快捷键或与系统快捷键冲突导致无法触发
2. QDockWidget 嵌套时恢复状态异常
3. 工具栏浮动后关闭导致丢失，需通过菜单重新显示

练习项目：实现一个简易文本编辑器主窗口，包含完整的菜单系统、
工具栏（新建/打开/保存/撤销/重做）、状态栏显示行列号、
可停靠的文件浏览器面板。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/07-mainwindow-framework-beginner.md
- examples/beginner/03-qtwidgets/07-mainwindow-framework-beginner/

## 参考资料

- [QMainWindow Class Reference](https://doc.qt.io/qt-6/qmainwindow.html)
- [QAction Class Reference](https://doc.qt.io/qt-6/qaction.html)
- [Main Window Examples](https://doc.qt.io/qt-6/examples-mainwindow.html)
