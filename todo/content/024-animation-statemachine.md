---
id: "024"
title: "QtWidgets 入门：动画框架 (QPropertyAnimation/状态机)"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: []
estimated_effort: medium
---

## 目标

掌握 Qt 动画框架的使用，包括 QPropertyAnimation、QSequentialAnimationGroup、
QParallelAnimationGroup 的组合使用。了解 QStateMachine 状态机框架，
学会构建具有流畅动画效果的交互界面。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QPropertyAnimation：属性动画基础
  - 目标对象与属性名
  - 动画时长与缓动曲线 (QEasingCurve)
  - 起始值与结束值
  - 关键帧 (setKeyValueAt)
- QAnimationGroup：串行动画组与并行动画组
- QEasingCurve：线性、弹性、回弹等各类缓动
- QStateMachine / QState 状态机
  - 状态定义与转换 (addTransition)
  - 动画转换 (QSignalTransition, QEventTransition)
  - 状态进入/退出动作
- QPauseAnimation 与延时控制

踩坑重点：
1. 动画属性未声明为 Q_PROPERTY 导致 QPropertyAnimation 无法工作
2. 动画未设置 stop() 就重新 start() 导致状态混乱
3. 状态机循环转换未设置条件导致无限切换

练习项目：实现一个侧边栏滑动展开/收起动画，配合状态机管理导航页面的切换过渡效果。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/09-animation-statemachine-beginner.md
- examples/beginner/03-qtwidgets/09-animation-statemachine-beginner/

## 参考资料

- [The Animation Framework](https://doc.qt.io/qt-6/animation-overview.html)
- [QPropertyAnimation Class Reference](https://doc.qt.io/qt-6/qpropertyanimation.html)
- [The State Machine Framework](https://doc.qt.io/qt-6/statemachine.html)
