---
id: "042"
title: "其他模块：Qt 3D 三维渲染"
category: content
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: large
---

## 目标

了解 Qt 3D 模块的基本使用，掌握三维场景的构建方法，包括实体 (Entity)、
组件 (Component) 架构、材质、光照、相机、网格等核心概念。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- Qt 3D 架构：ECS (Entity-Component-System) 模式
- Qt3DCore::QEntity 与 Qt3DCore::QComponent
- Qt3DRender 命名空间：
  - QCamera / QCameraLens：相机与投影
  - QRenderSettings / QForwardRenderer：渲染设置
  - QMesh / QCylinderMesh / QSphereMesh / QPlaneMesh / QCuboidMesh：网格
  - QPhongMaterial / QDiffuseSpecularMaterial：材质
  - QDirectionalLight / QPointLight / QSpotLight：光照
- Qt3DExtras：便捷类
  - Qt3DExtras::Qt3DWindow
  - OrbitCameraController：相机控制器
- 场景图构建
- 变换组件：QTransform (平移、旋转、缩放)
- 自定义材质与着色器 (QShaderProgram)
- 动画：QAnimationAspect

踩坑重点：
1. Qt 3D 与 QOpenGLWidget 是两套独立系统，不能直接混用
2. 场景中缺少光源导致所有物体渲染为纯黑
3. Qt 3D 在部分 Qt 6 版本中模块状态不稳定

练习项目：使用 Qt 3D 构建一个简单的 3D 场景展示器，
包含多个几何体、光照效果、相机旋转控制、材质切换功能。

## 涉及文件

- document/tutorials/beginner/05-other-modules/05-qt3d-rendering-beginner.md
- examples/beginner/05-other-modules/05-qt3d-rendering-beginner/

## 参考资料

- [Qt 3D Module](https://doc.qt.io/qt-6/qt3d-index.html)
- [Qt 3D Overview](https://doc.qt.io/qt-6/qt3doverview-module.html)
- [Qt 3D Basic Shapes Example](https://doc.qt.io/qt-6/qt3d-basicshapes-cpp-example.html)
