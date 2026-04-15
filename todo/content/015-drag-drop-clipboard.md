---
id: "015"
title: "QtGui 入门：拖放操作与剪贴板"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["010"]
blocks: []
estimated_effort: medium
---

## 目标

掌握 Qt 拖放 (Drag and Drop) 操作的实现机制与剪贴板 (QClipboard) 的使用。
学会自定义 MIME 类型的拖放数据传输，理解拖放事件的完整生命周期。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- 拖放生命周期：dragEnterEvent, dragMoveEvent, dragLeaveEvent, dropEvent
- QDrag 与 QMimeData 的使用
- 启动拖拽：mousePressEvent + mouseMoveEvent 配合
- 自定义 MIME 类型数据传输
- 同应用 vs 跨应用拖放
- QClipboard：文本、图像、HTML 的复制与粘贴
- QMimeData 数据提取
- setAcceptDrops() 属性设置
- 拖放视觉效果自定义

踩坑重点：
1. 忘记 setAcceptDrops(true) 导致拖放事件不触发
2. dragEnterEvent 中未调用 acceptProposedAction() 导致无法 drop
3. 剪贴板操作在非主线程执行导致崩溃

练习项目：实现一个支持文件拖入显示、文本拖放排序的列表控件，支持剪贴板复制粘贴。

## 涉及文件

- document/tutorials/beginner/02-qtgui/06-drag-drop-clipboard-beginner.md
- examples/beginner/02-qtgui/06-drag-drop-clipboard-beginner/

## 参考资料

- [Drag and Drop in Qt](https://doc.qt.io/qt-6/dnd.html)
- [QDrag Class Reference](https://doc.qt.io/qt-6/qdrag.html)
- [QClipboard Class Reference](https://doc.qt.io/qt-6/qclipboard.html)
