---
id: "027"
title: "QtWidgets 控件速查：按钮家族 (QPushButton/QCheckBox/QRadioButton/QToolButton)"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: []
estimated_effort: medium
---

## 目标

全面掌握 Qt 按钮家族控件的使用，包括 QPushButton、QCheckBox、QRadioButton、QToolButton
以及 QButtonGroup 的分组管理。理解各类按钮的适用场景与高级特性。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QPushButton：标准按钮
  - default/autoDefault 属性
  - 菜单关联 (setMenu)
  - flat 样式
  - 快捷键设置
- QCheckBox：复选框
  - 三态模式 (setTristate): Checked, Unchecked, PartiallyChecked
  - checkStateChanged 信号
- QRadioButton：单选按钮
  - 互斥机制与 autoExclusive
- QToolButton：工具按钮
  - setToolButtonStyle: IconOnly, TextOnly, TextBesideIcon, TextUnderIcon
  - 关联 QMenu 与弹出模式 (setPopupMode)
  - autoRaise 样式
  - setDefaultAction 关联动作
- QButtonGroup：按钮分组管理
  - 分组互斥逻辑
  - id-based 管理：button, checkedId, checkedButton
  - 信号：idClicked, idToggled, buttonClicked

踩坑重点：
1. 多组 QRadioButton 未用 QButtonGroup 隔离导致全局互斥
2. QPushButton::setMenu 后点击按钮弹出菜单但不触发 clicked 信号
3. QToolButton 的 arrowType 与 icon 同时设置时图标被覆盖

练习项目：实现一个设置面板，综合使用所有按钮类型：
普通按钮执行操作、复选框切换选项、单选按钮选择模式、
工具按钮带下拉菜单，使用 QButtonGroup 管理单选互斥。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/controls/02-button-family.md
- examples/beginner/03-qtwidgets/controls/02-button-family/

## 参考资料

- [QPushButton Class Reference](https://doc.qt.io/qt-6/qpushbutton.html)
- [QCheckBox Class Reference](https://doc.qt.io/qt-6/qcheckbox.html)
- [QRadioButton Class Reference](https://doc.qt.io/qt-6/qradiobutton.html)
- [QToolButton Class Reference](https://doc.qt.io/qt-6/qtoolbutton.html)
- [QButtonGroup Class Reference](https://doc.qt.io/qt-6/qbuttongroup.html)
