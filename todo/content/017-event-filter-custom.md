---
id: "017"
title: "QtWidgets 入门：事件过滤器与自定义事件"
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

深入理解 Qt 事件系统的工作原理，掌握事件过滤器 (eventFilter) 的使用，
学会创建和发送自定义事件 (QEvent)。理解事件传播链：事件产生、分发、过滤、处理的完整流程。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- Qt 事件循环与事件类型 (QEvent::Type)
- 事件处理函数：event(), mousePressEvent(), keyPressEvent() 等
- installEventFilter() 与 eventFilter() 机制
- 事件传播：accept() / ignore() / isAccepted()
- 自定义事件：QEvent 子类化与 registerEventType()
- postEvent() vs sendEvent() 的区别
- QCoreApplication::notify() 的角色
- 事件压缩与合并

踩坑重点：
1. eventFilter 返回 false 后事件仍然被过滤掉（应为 true 才表示已处理）
2. 在 event() 中忘记调用父类实现导致其他事件丢失
3. postEvent 使用栈上 QEvent 对象导致悬空指针崩溃

练习项目：实现一个全局快捷键监听器，通过事件过滤器捕获组合键并触发自定义事件。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/02-event-filter-custom-beginner.md
- examples/beginner/03-qtwidgets/02-event-filter-custom-beginner/

## 参考资料

- [The Event System](https://doc.qt.io/qt-6/eventsandfilters.html)
- [QEvent Class Reference](https://doc.qt.io/qt-6/qevent.html)
- [QObject::installEventFilter](https://doc.qt.io/qt-6/qobject.html#installEventFilter)
