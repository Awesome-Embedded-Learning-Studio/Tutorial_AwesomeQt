---
id: "031"
title: "QtWidgets 控件速查：视图控件 (QListView/QTreeView/QTableView/QColumnView)"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["018"]
blocks: []
estimated_effort: large
---

## 目标

全面掌握 Qt 视图控件的使用，包括 QListView、QTreeView、QTableView、QColumnView
以及对应的基本使用模式。理解各类视图控件的特性与适用场景，掌握视图定制化能力。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QListView：列表视图
  - 基本使用：setModel, currentItem, currentIndex
  - 显示模式：ListMode, IconMode
  - 布局方向、换行模式
  - 选择模式：SingleSelection, MultiSelection, ExtendedSelection, ContiguousSelection
  - 编辑触发：EditTrigger 枚举
- QTreeView：树形视图
  - 展开/折叠：expand, collapse, expandAll, collapseAll
  - setRootIsDecorated, setItemsExpandable
  - 表头控制：header()->hide()
  - 缩进与视觉反馈
  - 拖放排序
- QTableView：表格视图
  - 行列操作：hideColumn, hideRow, resizeColumnToContents
  - 表头定制：horizontalHeader, verticalHeader
  - 网格线与交替行色
  - 单元格编辑
  - 排序与过滤
- QColumnView：列视图（Mac 风格浏览器）
  - 分栏显示模式
  - 预览部件 setPreviewWidget

踩坑重点：
1. QListView 在 IconMode 下图片过大导致布局混乱
2. QTreeView 数据量大时 expandAll() 导致严重卡顿
3. QTableView 自定义 Delegate 编辑器关闭时未正确提交数据

练习项目：实现一个文件管理器界面，左侧 QTreeView 显示目录树、
右侧 QTableView 显示当前目录文件详情、支持切换为 QListView 图标模式浏览。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/controls/06-view-widgets.md
- examples/beginner/03-qtwidgets/controls/06-view-widgets/

## 参考资料

- [QListView Class Reference](https://doc.qt.io/qt-6/qlistview.html)
- [QTreeView Class Reference](https://doc.qt.io/qt-6/qtreeview.html)
- [QTableView Class Reference](https://doc.qt.io/qt-6/qtableview.html)
- [QColumnView Class Reference](https://doc.qt.io/qt-6/qcolumnview.html)
