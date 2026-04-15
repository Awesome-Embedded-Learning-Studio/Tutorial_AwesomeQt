---
id: "016"
title: "QtWidgets 入门：布局管理系统"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["010"]
blocks: ["017", "018", "019", "020", "021", "022", "023", "024", "025", "026", "027", "028", "029", "030", "046"]
estimated_effort: medium
---

## 目标

深入掌握 Qt 布局管理系统，包括 QHBoxLayout、QVBoxLayout、QGridLayout、QFormLayout
以及 QStackedLayout 的使用。理解尺寸策略 (QSizePolicy)、拉伸因子、间距与边距的概念，
能够构建灵活响应式的界面布局。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- 四大布局类：QHBoxLayout, QVBoxLayout, QGridLayout, QFormLayout, QStackedLayout
- addWidget(), addLayout(), addStretch(), addSpacing()
- QSizePolicy：Fixed, Minimum, Maximum, Preferred, Expanding, MinimumExpanding, Ignored
- 拉伸因子 (stretch factor) 与空间分配
- 间距 (spacing) 与边距 (margin/contentsMargins)
- 嵌套布局的构建模式
- sizeHint() 与 minimumSizeHint()
- 布局激活与失效 (activate/invalidate)
- QSpacerItem 与空白区域控制

踩坑重点：
1. 同时设置固定大小和布局拉伸导致布局无法正常工作
2. 嵌套布局忘记设置外层布局导致控件重叠
3. sizeHint 返回无效值导致控件大小为 0

练习项目：实现一个响应式登录表单界面，使用多种布局组合，支持窗口缩放。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/01-layout-management-beginner.md
- examples/beginner/03-qtwidgets/01-layout-management-beginner/

## 参考资料

- [Layout Management](https://doc.qt.io/qt-6/layout.html)
- [QBoxLayout Class Reference](https://doc.qt.io/qt-6/qboxlayout.html)
- [QGridLayout Class Reference](https://doc.qt.io/qt-6/qgridlayout.html)
- [QFormLayout Class Reference](https://doc.qt.io/qt-6/qformlayout.html)
