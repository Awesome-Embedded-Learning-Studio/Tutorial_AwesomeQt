---
id: "047"
title: "QML 入门：组件与属性绑定"
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

掌握 QML 组件的创建与复用机制，深入理解属性绑定 (Property Binding) 的工作原理。
学会自定义组件开发、信号与属性的传递、组件生命周期管理。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QML 组件基础：
  - 组件即文件：一个 .qml 文件即一个组件
  - 组件命名规则 (大写字母开头)
  - 组件的实例化与复用
- 属性绑定：
  - 绑定表达式：width: parent.width / 2
  - Binding 元素
  - Qt.binding() 函数
  - 绑定断裂与修复 (break binding)
  - 双向绑定 (Binding on property)
- 自定义属性与接口设计：
  - 输入属性 (property declarations)
  - 输出信号 (signal declarations)
  - 属性别名暴露内部控件
- 信号与回调：
  - 自定义信号声明
  - 信号参数传递
  - Connections 元素
- 组件生命周期：
  - Component.onCompleted / Component.onDestruction
  - Loader 动态加载与卸载
  - 异步加载
- QML 模块 (qmldir) 基础

踩坑重点：
1. JavaScript 赋值破坏属性绑定导致后续更新不生效
2. 组件内部属性未通过 alias 暴露导致外部无法访问
3. Loader 加载的组件中直接访问 parent 为 null

练习项目：实现一套可复用的自定义 UI 组件库，包含自定义按钮、
输入框、卡片组件，通过属性绑定实现主题切换 (颜色/字号自动适配)。

## 涉及文件

- document/tutorials/beginner/06-qml/02-qml-components-bindings-beginner.md
- examples/beginner/06-qml/02-qml-components-bindings-beginner/

## 参考资料

- [QML Components](https://doc.qt.io/qt-6/qtqml-components-topic.html)
- [Property Binding](https://doc.qt.io/qt-6/qtqml-syntax-propertybinding.html)
- [QML Object Attributes](https://doc.qt.io/qt-6/qtqml-syntax-objectattributes.html)
