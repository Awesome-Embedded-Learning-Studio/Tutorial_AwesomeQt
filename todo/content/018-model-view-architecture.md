---
id: "018"
title: "QtWidgets 入门：模型/视图架构 (MVC)"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: ["031", "049"]
estimated_effort: large
---

## 目标

深入理解 Qt 的模型/视图架构，掌握 QAbstractItemModel、QAbstractItemView、
QAbstractItemDelegate 三者的协作关系。学会使用内置模型 (QStringListModel、
QStandardItemModel、QFileSystemModel) 以及自定义模型的开发。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- Model/View/Delegate 三者职责划分
- QModelIndex 与 model index 的概念
- 内置模型：QStringListModel, QStandardItemModel, QFileSystemModel
- 自定义模型：QAbstractItemModel 子类化
  - rowCount(), columnCount(), data(), index(), parent()
  - 只读模型 vs 可编辑模型 (flags + setData)
- 选择模型：QItemSelectionModel
- Delegate 基础：QStyledItemDelegate 子类化
  - paint(), sizeHint(), createEditor(), setEditorData(), setModelData()
- 数据角色：Qt::DisplayRole, Qt::EditRole, Qt::DecorationRole 等
- QSortFilterProxyModel 排序与过滤

踩坑重点：
1. createIndex() 使用非法内部指针导致模型崩溃
2. data() 函数忘记处理不同 role 导致显示异常
3. beginInsertRows/endInsertRows 未配对使用导致视图不更新

练习项目：实现一个自定义树形模型，展示公司组织架构（部门-团队-成员三级结构），
支持节点增删改查与拖拽排序。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/03-model-view-mvc-beginner.md
- examples/beginner/03-qtwidgets/03-model-view-mvc-beginner/

## 参考资料

- [Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html)
- [QAbstractItemModel Class Reference](https://doc.qt.io/qt-6/qabstractitemmodel.html)
- [QStyledItemDelegate Class Reference](https://doc.qt.io/qt-6/qstyleditemdelegate.html)
