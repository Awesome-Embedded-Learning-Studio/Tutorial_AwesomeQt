---
id: "048"
title: "QML 入门：布局系统与锚点"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["046"]
blocks: []
estimated_effort: medium
---

## 目标

全面掌握 QML 布局系统，包括锚点 (Anchors) 定位、Row/Column/Grid/Flow 定位器、
Layout 布局器的使用。理解各布局方式的适用场景，能够构建灵活响应式的 QML 界面。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- 锚点布局 (Anchors)：
  - anchors.fill, anchors.centerIn
  - anchors.left/right/top/bottom + margins
  - anchors.horizontalCenter/verticalCenter
  - anchors.baseline
  - 锚点对齐：anchors.left: sibling.right
  - 锚点不能跨父级兄弟的限制
- 定位器 (Positioners)：
  - Row / Column：水平/垂直排列
  - Grid：网格排列
  - Flow：流式排列 (自动换行)
  - spacing, padding, layoutDirection
  - add / move / populate 过渡动画
- 布局器 (Layouts)：
  - RowLayout / ColumnLayout / GridLayout (Qt Quick Layouts)
  - Layout.fillWidth / Layout.fillHeight
  - Layout.preferredWidth / Layout.preferredHeight
  - Layout.minimumWidth / Layout.maximumWidth
  - Layout.alignment / Layout.margins
  - 拉伸因子
- x/y 坐标定位 (绝对定位)
- Item 作为容器与布局根元素

踩坑重点：
1. 锚点与 x/y/width/height 同时设置导致冲突
2. Layouts 中未设置 fillWidth 导致子项宽度不跟随父容器
3. 锚点循环依赖导致绑定错误

练习项目：使用 QML 实现一个响应式仪表盘界面，综合使用锚点定位、
RowLayout/ColumnLayout/Grid 布局，包含头部导航栏、侧边栏、
内容区域，支持窗口缩放时自动适配。

## 涉及文件

- document/tutorials/beginner/06-qml/03-qml-layout-anchors-beginner.md
- examples/beginner/06-qml/03-qml-layout-anchors-beginner/

## 参考资料

- [Qt Quick Layouts Overview](https://doc.qt.io/qt-6/qtquicklayouts-index.html)
- [Positioners in Qt Quick](https://doc.qt.io/qt-6/qtquick-positioners-topic.html)
- [Anchor Layouts](https://doc.qt.io/qt-6/qtquick-positioning-anchors.html)
