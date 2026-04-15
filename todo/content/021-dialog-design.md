---
id: "021"
title: "QtWidgets 入门：对话框设计 (标准/自定义/向导)"
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

掌握 Qt 对话框的各类使用方式，包括标准对话框 (QMessageBox, QFileDialog,
QColorDialog, QFontDialog, QInputDialog)、自定义对话框的设计与使用、
以及向导对话框 (QWizard) 的构建。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QDialog 模态与非模态 (exec() vs show() + setWindowModality)
- 标准对话框：
  - QMessageBox：信息、问答、警告、错误对话框
  - QFileDialog：文件打开/保存
  - QColorDialog：颜色选择
  - QFontDialog：字体选择
  - QInputDialog：简单输入 (文本、整数、浮点数、列表选择)
- 自定义对话框设计模式
  - 属性获取模式：构建对话框 -> exec() -> 获取结果
  - 数据验证与 accept/reject 逻辑
- QWizard 与 QWizardPage：
  - 页面注册与字段机制
  - 页面跳转逻辑
  - 自定义验证
- Qt::DialogFlag 窗口标志

踩坑重点：
1. exec() 在嵌套事件循环中导致重入问题
2. QFileDialog 静态方法不支持自定义过滤器组合
3. QWizard 字段注册名称与 UI 控件不匹配导致数据丢失

练习项目：实现一个软件安装向导，包含欢迎页、许可协议页、安装路径选择页、
组件选择页、安装进度页和完成页。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/06-dialog-design-beginner.md
- examples/beginner/03-qtwidgets/06-dialog-design-beginner/

## 参考资料

- [QDialog Class Reference](https://doc.qt.io/qt-6/qdialog.html)
- [QWizard Class Reference](https://doc.qt.io/qt-6/qwizard.html)
- [Standard Dialogs Example](https://doc.qt.io/qt-6/qtwidgets-dialogs-standarddialogs-example.html)
