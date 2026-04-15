---
id: "030"
title: "QtWidgets 控件速查：容器控件 (QGroupBox/QTabWidget/QSplitter/QScrollArea)"
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

全面掌握 Qt 容器控件的使用，包括 QGroupBox、QTabWidget/QTabBar、
QSplitter、QScrollArea、QToolBox 等控件。理解容器控件在界面组织和内容分组中的作用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QGroupBox：分组框
  - 标题与对齐 (setTitle, setAlignment)
  - 可勾选 (setCheckable)
  - flat 样式
  - 内部布局管理
- QTabWidget / QTabBar：标签页
  - addTab / insertTab / removeTab
  - 页面图标与工具提示
  - 标签位置：North, South, West, East
  - 标签形状：Rounded, Triangular
  - 可关闭标签 (tabsClosable)
  - currentChanged 信号
  - QTabWidget vs QTabBar 的选择
- QSplitter：分割器
  - addWidget / insertWidget
  - setSizes / sizes
  - setStretchFactor
  - 方向：水平/垂直
  - 可折叠 (childrenCollapsible)
  - 嵌套分割器
  - saveState / restoreState
- QScrollArea：滚动区域
  - setWidget / takeWidget
  - widgetResizable
  - 滚动条策略
  - 自定义滚动区域内容
- QToolBox：工具箱

踩坑重点：
1. QScrollArea::setWidget 后未设置 widgetResizable=true 导致内容不缩放
2. QSplitter::setSizes 的参数总和与实际不符导致比例不对
3. QTabWidget 动态删除页面时索引越界

练习项目：实现一个设置对话框，使用 QGroupBox 分组各设置项、
QTabWidget 分类不同设置面板、QSplitter 分割预览与编辑区域、
QScrollArea 包裹过长的设置列表。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/controls/05-container-widgets.md
- examples/beginner/03-qtwidgets/controls/05-container-widgets/

## 参考资料

- [QGroupBox Class Reference](https://doc.qt.io/qt-6/qgroupbox.html)
- [QTabWidget Class Reference](https://doc.qt.io/qt-6/qtabwidget.html)
- [QSplitter Class Reference](https://doc.qt.io/qt-6/qsplitter.html)
- [QScrollArea Class Reference](https://doc.qt.io/qt-6/qscrollarea.html)
