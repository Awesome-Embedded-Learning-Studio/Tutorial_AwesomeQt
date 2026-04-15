---
id: "040"
title: "其他模块：QtMultimedia 音视频"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: large
---

## 目标

掌握 QtMultimedia 模块的使用，包括音频播放/录制、视频播放、
摄像头捕获等功能。理解 Qt 6 中 Multimedia 模块的架构变化与核心类。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- Qt 6 Multimedia 架构变化（vs Qt 5）
- QMediaPlayer：媒体播放
  - setSource / setAudioOutput
  - play / pause / stop
  - position / duration / seek
  - 播放状态与错误处理
- QAudioOutput：音频输出设备
- QAudioInput：音频输入设备
- QMediaCaptureSession：捕获会话
- QCamera：摄像头
  - 设备枚举
  - captureSession 集成
  - 图片捕获 (QImageCapture)
  - 视频录制 (QMediaRecorder)
- QVideoWidget：视频显示组件
- QMediaFormat 与编解码器支持
- QSoundEffect：简短音效播放
- CMake 配置：find_package(Qt6 REQUIRED COMPONENTS Multimedia MultimediaWidgets)

踩坑重点：
1. Qt 6 Multimedia API 与 Qt 5 完全不同，旧教程代码无法直接使用
2. 缺少系统编解码器导致某些格式无法播放
3. 摄像头权限未授予导致无法打开设备

练习项目：实现一个多媒体播放器，支持音频和视频播放，
包含播放列表管理、进度控制、音量调节，以及摄像头拍照功能。

## 涉及文件

- document/tutorials/beginner/05-other-modules/03-qtmultimedia-beginner.md
- examples/beginner/05-other-modules/03-qtmultimedia-beginner/

## 参考资料

- [Qt Multimedia Module](https://doc.qt.io/qt-6/qtmultimedia-index.html)
- [QMediaPlayer Class Reference](https://doc.qt.io/qt-6/qmediaplayer.html)
- [QCamera Class Reference](https://doc.qt.io/qt-6/qcamera.html)
