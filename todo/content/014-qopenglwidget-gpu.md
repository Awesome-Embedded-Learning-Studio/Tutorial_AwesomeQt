---
id: "014"
title: "QtGui 入门：QOpenGLWidget 与 GPU 渲染"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["010"]
blocks: []
estimated_effort: large
---

## 目标

理解 Qt 中 OpenGL 集成的基本方式，掌握 QOpenGLWidget 的使用，
学会利用 GPU 进行加速渲染。了解 QOpenGLFunctions、着色器编程基础，
以及 OpenGL 与 Qt Widget 系统的混合使用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QOpenGLWidget 生命周期：initializeGL, resizeGL, paintGL
- QOpenGLFunctions 基础 OpenGL 调用
- QOpenGLShaderProgram 着色器编译与链接
- 顶点着色器与片段着色器基础
- VBO/VAO 概念与使用
- 纹理加载与映射
- QOpenGLContext 与上下文管理
- 与 QPainter 的混合使用 (beginNativePainting / endNativePainting)
- 帧缓冲对象 (FBO) 基础

踩坑重点：
1. 在 paintGL 之外调用 OpenGL 函数导致未定义行为
2. 着色器编译失败但未检查日志，渲染出空白画面
3.忘记释放 VBO/VAO 等 GPU 资源导致内存泄漏

练习项目：使用 QOpenGLWidget 绘制一个带纹理的旋转立方体。

## 涉及文件

- document/tutorials/beginner/02-qtgui/05-qopenglwidget-gpu-beginner.md
- examples/beginner/02-qtgui/05-qopenglwidget-gpu-beginner/

## 参考资料

- [QOpenGLWidget Class Reference](https://doc.qt.io/qt-6/qopenglwidget.html)
- [Qt OpenGL Module](https://doc.qt.io/qt-6/qtopengl-index.html)
- [QOpenGLShaderProgram Class Reference](https://doc.qt.io/qt-6/qopenglshaderprogram.html)
