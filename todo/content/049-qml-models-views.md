---
id: "049"
title: "QML 入门：模型与视图"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["046", "018"]
blocks: []
estimated_effort: medium
---

## 目标

掌握 QML 中模型与视图的使用，包括 ListModel、ListView、GridView、
Delegate 的设计。理解 QML 数据驱动 UI 的模式，以及 C++ 模型与 QML 视图的集成。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QML 数据模型：
  - ListModel + ListElement：纯 QML 列表模型
  - model.get(index), model.append(), model.set(), model.remove()
  - 动态角色 (Dynamic Roles)
- 视图控件：
  - ListView：列表视图
    - orientation, spacing, snapMode
    - cacheBuffer, highlight, highlightFollowsCurrentItem
    - header / footer / section
    - currentIndex, currentItem
  - GridView：网格视图
    - cellWidth, cellHeight, flow
  - PathView：路径视图
    - path, pathItemCount, preferredHighlightBegin/End
- Delegate 代理设计：
  - 控件复用与性能
  - itemWidth/itemHeight
  - 包装代理 (wrapper delegate) 模式
  - Binding 与 Required Properties
- C++ 模型集成：
  - QStringList
  - QVariantList / QVariantMap
  - QAbstractItemModel 子类暴露给 QML
  - QAbstractListModel 的角色 (roleNames)
- 性能优化：
  - 异步加载
  - 减少 delegate 复杂度
  - Loader 按需加载

踩坑重点：
1. ListModel 动态修改后视图未刷新，需要正确的 beginInsertRows/endInsertRows
2. Delegate 中过度使用 Binding 导致绑定循环警告
3. 大列表未设置 cacheBuffer 导致快速滚动时出现空白区域

练习项目：实现一个联系人管理应用，使用 ListView 展示联系人列表，
自定义 Delegate 显示头像、姓名、电话，支持字母分组 (section)、
搜索过滤、侧边字母索引快速定位。

## 涉及文件

- document/tutorials/beginner/06-qml/04-qml-models-views-beginner.md
- examples/beginner/06-qml/04-qml-models-views-beginner/

## 参考资料

- [Qt Quick Views](https://doc.qt.io/qt-6/qtquick-modelviewsdata-modelview.html)
- [ListView Class Reference](https://doc.qt.io/qt-6/qml-qtquick-listview.html)
- [ListModel Class Reference](https://doc.qt.io/qt-6/qml-qtqml-models-listmodel.html)
- [Models and Views in Qt Quick](https://doc.qt.io/qt-6/qtquick-models-topic.html)
