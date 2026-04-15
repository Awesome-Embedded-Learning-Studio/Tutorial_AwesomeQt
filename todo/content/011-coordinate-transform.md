---
id: "011"
title: "QtGui 入门：坐标变换"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["010"]
blocks: []
estimated_effort: medium
---

## 目标

掌握 QPainter 提供的坐标变换能力，包括平移 (translate)、旋转 (rotate)、缩放 (scale)
以及通用仿射变换 (QTransform)。理解坐标系变换栈的工作原理，
能够利用变换简化复杂绘图逻辑，实现图形的旋转、镜像、斜切等效果。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- 坐标系基础：逻辑坐标 vs 物理坐标
- translate() 平移变换
- rotate() 旋转变换（注意角度单位为度）
- scale() 缩放变换
- QTransform 通用仿射变换矩阵
- save()/restore() 状态栈管理
- 变换的组合顺序及其对结果的影响
- setTransform() vs 增量变换
- 视口 (viewport) 与窗口 (window) 映射

踩坑重点：
1. rotate() 以原点为中心旋转，忘记先 translate 到目标中心
2. 变换顺序不可交换：先旋转后平移 vs 先平移后旋转结果完全不同
3. 忘记 restore() 导致后续绘制全部受到变换影响

练习项目：绘制一个带有时针、分针、秒针的模拟时钟，利用坐标变换实现指针旋转。

## 涉及文件

- document/tutorials/beginner/02-qtgui/02-coordinate-transform-beginner.md
- examples/beginner/02-qtgui/02-coordinate-transform-beginner/

## 参考资料

- [QPainter Coordinate System](https://doc.qt.io/qt-6/coordsys.html)
- [QTransform Class Reference](https://doc.qt.io/qt-6/qtransform.html)
