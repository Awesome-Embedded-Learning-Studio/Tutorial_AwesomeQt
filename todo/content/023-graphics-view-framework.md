---
id: "023"
title: "QtWidgets 入门：图形视图框架 (QGraphicsScene/View)"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["010", "016"]
blocks: []
estimated_effort: large
---

## 目标

掌握 Qt 图形视图框架 (Graphics View Framework) 的架构与使用，
包括 QGraphicsScene、QGraphicsView、QGraphicsItem 三者的协作。
学会构建交互式 2D 图形场景，理解坐标系映射、事件传播、碰撞检测等核心机制。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- Graphics View 三件套：QGraphicsScene, QGraphicsView, QGraphicsItem
- 预定义图元：QGraphicsRectItem, QGraphicsEllipseItem, QGraphicsTextItem, QGraphicsPixmapItem, QGraphicsPathItem
- 自定义 QGraphicsItem 子类：boundingRect() 与 paint()
- 三套坐标系：场景坐标、视图坐标、图元坐标
- 坐标映射：mapToScene, mapFromScene, mapToParent, mapFromParent
- 事件处理：QGraphicsItem 的事件传播链
- 碰撞检测：collidingItems(), shape(), contains()
- 图元分组：QGraphicsItemGroup
- 视图变换：缩放、旋转
- 场景索引与性能优化

踩坑重点：
1. boundingRect() 返回的区域小于实际绘制区域导致裁剪
2. 自定义 QGraphicsItem 忘记调用 prepareGeometryChange() 导致渲染残留
3. 场景坐标与视图坐标混淆导致交互位置偏移

练习项目：实现一个简易流程图编辑器，支持拖放创建节点、连线、节点移动、缩放视图。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/08-graphics-view-beginner.md
- examples/beginner/03-qtwidgets/08-graphics-view-beginner/

## 参考资料

- [Graphics View Framework](https://doc.qt.io/qt-6/graphicsview.html)
- [QGraphicsScene Class Reference](https://doc.qt.io/qt-6/qgraphicsscene.html)
- [QGraphicsItem Class Reference](https://doc.qt.io/qt-6/qgraphicsitem.html)
