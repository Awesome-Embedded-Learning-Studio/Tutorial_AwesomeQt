---
id: "029"
title: "QtWidgets 控件速查：显示控件 (QLabel/QProgressBar/QStatusBar)"
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

全面掌握 Qt 显示控件的使用，包括 QLabel、QProgressBar、QStatusBar
以及相关的显示辅助类。理解这些控件在信息展示和用户反馈中的作用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QLabel：标签控件
  - 文本显示：纯文本、富文本 (HTML)、自动换行
  - 图片显示：setPixmap, setMovie (QMovie 动图)
  - 对齐方式：setAlignment
  - 交互：setOpenExternalLinks, linkActivated 信号
  - 伙伴机制：setBuddy (与快捷键配合)
  - 文本省略：elide 模式
- QProgressBar：进度条
  - 进度范围与当前值
  - 方向：水平/垂直
  - 文本可见性与自定义格式 (setFormat)
  - 不确定模式 (setRange(0, 0))
  - 反向进度 (invertedAppearance)
- QStatusBar：状态栏
  - showMessage / clearMessage 临时消息
  - addWidget / addPermanentWidget 永久部件
  - sizeGripEnabled 右下角调整手柄
- QLCDNumber：数字显示
- QMovie：GIF 动画播放

踩坑重点：
1. QLabel 显示大图未设置缩放导致控件撑破布局
2. QProgressBar 在不确定模式下仍显示百分比文本
3. QStatusBar::showMessage 被永久部件遮挡显示不全

练习项目：实现一个文件下载进度面板，包含 QLabel 显示文件名与图标、
QProgressBar 显示下载进度、QStatusBar 显示实时速度与剩余时间。

## 涉及文件

- document/tutorials/beginner/03-qtwidgets/controls/04-display-widgets.md
- examples/beginner/03-qtwidgets/controls/04-display-widgets/

## 参考资料

- [QLabel Class Reference](https://doc.qt.io/qt-6/qlabel.html)
- [QProgressBar Class Reference](https://doc.qt.io/qt-6/qprogressbar.html)
- [QStatusBar Class Reference](https://doc.qt.io/qt-6/qstatusbar.html)
