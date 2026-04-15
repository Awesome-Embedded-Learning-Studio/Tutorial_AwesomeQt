---
id: "028"
title: "QtWidgets 控件速查：输入控件 (QLineEdit/QTextEdit/QSpinBox/QComboBox)"
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

全面掌握 Qt 输入控件的使用，包括 QLineEdit、QTextEdit/QPlainTextEdit、
QSpinBox/QDoubleSpinBox、QComboBox 以及输入验证与格式化机制。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QLineEdit：单行文本输入
  - 输入掩码 (setInputMask)
  - 验证器：QIntValidator, QDoubleValidator, QRegularExpressionValidator
  - 回显模式 (echoMode)：Normal, Password, NoEcho, PasswordEchoOnEdit
  - 补全器 (QCompleter)
  - 撤销/重做、选中文本操作
  - textChanged vs textEdited 信号区别
- QTextEdit / QPlainTextEdit：多行文本
  - 纯文本 vs 富文本 (HTML)
  - QTextCursor 与文本操作
  - 语法高亮 (QSyntaxHighlighter 简介)
- QSpinBox / QDoubleSpinBox：数值输入
  - 范围、步长、前缀、后缀
  - 自定义文本显示 (textFromValue)
- QComboBox：下拉选择
  - 可编辑模式
  - 模型驱动：setModel
  - addItem / insertItem / currentIndexChanged

踩坑重点：
1. QLineEdit 验证器允许中间输入状态但保存了无效值
2. QTextEdit toPlainText() vs toHtml() 在富文本模式下返回不同内容
3. QComboBox 可编辑模式下 currentTextChanged 信号频繁触发

练习项目：实现一个用户信息录入表单，综合使用各类输入控件，
包含输入验证、密码输入、数值范围限制、下拉选择等功能。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/controls/03-input-widgets.md
- examples/beginner/03-qtwidgets/controls/03-input-widgets/

## 参考资料

- [QLineEdit Class Reference](https://doc.qt.io/qt-6/qlineedit.html)
- [QTextEdit Class Reference](https://doc.qt.io/qt-6/qtextedit.html)
- [QSpinBox Class Reference](https://doc.qt.io/qt-6/qspinbox.html)
- [QComboBox Class Reference](https://doc.qt.io/qt-6/qcombobox.html)
- [QValidator Class Reference](https://doc.qt.io/qt-6/qvalidator.html)
