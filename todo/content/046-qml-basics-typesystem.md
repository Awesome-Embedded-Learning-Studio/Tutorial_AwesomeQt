---
id: "046"
title: "QML 入门：QML 基础语法与类型系统"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: ["047", "048", "049"]
estimated_effort: medium
---

## 目标

掌握 QML 的基础语法与类型系统，理解 QML 声明式编程范式。
学会 QML 文档结构、基本类型、对象声明、属性系统、信号处理、
JavaScript 集成等核心概念，建立 QML 开发的基础能力。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QML 文档结构：import 声明、根对象、子对象
- 基本类型：int, real, bool, string, color, date, url, var, variant, enumeration
- 对象声明与层级关系
- 属性系统：
  - 属性声明：property int myValue: 0
  - 属性别名：property alias
  - 默认属性：default property
  - 只读属性：readonly property
  - 属性变化信号：onXChanged
- 信号与处理器：signal / onSignal / Connections
- JavaScript 集成：
  - 内联 JavaScript 表达式
  - js 函数声明
  - 条件与循环
  - Qt.include() 外部 JS 文件
- 枚举类型与命名空间
- 注释语法
- 调试基础：console.log, console.debug

踩坑重点：
1. property alias 循环引用导致启动崩溃
2. var 类型滥用导致类型安全问题，运行时才暴露错误
3. JavaScript 表达式中使用未声明变量导致静默失败

练习项目：使用纯 QML 实现一个简单计算器界面，涵盖基本类型、属性绑定、
信号处理、JavaScript 逻辑，使用 QQmlApplicationEngine 加载。

## 涉及文件

- document/tutorials/beginner/06-qml/01-qml-basics-typesystem-beginner.md
- examples/beginner/06-qml/01-qml-basics-typesystem-beginner/

## 参考资料

- [QML Language Reference](https://doc.qt.io/qt-6/qtqml-qmllanguage.html)
- [QML Basic Types](https://doc.qt.io/qt-6/qtqml-typesystem-basictypes.html)
- [QML Object Attributes](https://doc.qt.io/qt-6/qtqml-syntax-objectattributes.html)
- [Integrating QML and C++](https://doc.qt.io/qt-6/qtqml-cppintegration-overview.html)
