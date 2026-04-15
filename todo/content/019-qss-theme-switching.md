---
id: "019"
title: "QtWidgets 入门：QSS 样式表与主题切换"
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

掌握 Qt 样式表 (QSS) 的语法与使用方法，学会为控件设置自定义样式，
实现主题切换功能。理解 QSS 选择器、属性、伪状态、子控件控制等核心概念。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QSS 语法基础：选择器、属性、值
- 选择器类型：通用、类型、类、ID、属性、后代、子选择器
- 伪状态：:hover, :pressed, :checked, :disabled, :focus 等
- 子控件：::indicator, ::menu-indicator, ::drop-down 等
- 盒模型：margin, border, padding, content
- 背景与渐变
- QPalette 与 QSS 的关系与优先级
- 动态主题切换实现模式
- 从文件加载 QSS

踩坑重点：
1. QSS 样式优先级混乱，多处设置互相覆盖难以排查
2. QSS gradient 语法与 CSS 不完全兼容导致渲染错误
3. 主题切换时未 unpolish/repolish 导致样式缓存未刷新

练习项目：实现一个支持深色/浅色主题切换的设置面板，包含多种控件类型的统一样式。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/04-qss-theme-switching-beginner.md
- examples/beginner/03-qtwidgets/04-qss-theme-switching-beginner/

## 参考资料

- [Qt Style Sheets Reference](https://doc.qt.io/qt-6/stylesheet-reference.html)
- [Qt Style Sheets Examples](https://doc.qt.io/qt-6/stylesheet-examples.html)
- [QPalette Class Reference](https://doc.qt.io/qt-6/qpalette.html)
