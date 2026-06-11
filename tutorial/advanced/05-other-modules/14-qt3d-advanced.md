---
title: "5.14 Qt3D 进阶：自定义 Material、Framegraph 配置"
description: "入门篇我们把 Qt3D 的基本 3D 场景跑通了——Entity + Mesh + Material + Camera，渲染一个带光照的 3D 物体。但入门篇用的都是 Qt3D 内置的默认材质和渲染管线。真正的 3D 应用需要自定义着色器材质（比如水面、玻璃、全息效果）和精细的渲染管线控制（多 Pass 渲染、后处理效果、阴影贴图）。"
---

# 现代Qt开发教程（进阶篇）5.14——Qt3D 进阶：自定义 Material、Framegraph 配置

## 1. 前言

入门篇我们把 Qt3D 的基本 3D 场景跑通了——Entity + Mesh + Material + Camera，渲染一个带光照的 3D 物体。但入门篇用的都是 Qt3D 内置的默认材质和渲染管线。真正的 3D 应用需要自定义着色器材质（比如水面、玻璃、全息效果）和精细的渲染管线控制（多 Pass 渲染、后处理效果、阴影贴图）。

这篇我们把自定义 Material（使用 GLSL 着色器）和 Framegraph（渲染管线配置）这两个核心能力拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::3DCore、Qt6::3DRender、Qt6::3DExtras 模块。需要 OpenGL 3.0+ 或 Vulkan 支持的 GPU。

## 3. 核心概念讲解

### 3.1 自定义 Material——QShaderProgram + QParameter

Qt3D 的 Material 系统基于「参数 + 着色器」的组合。你提供 GLSL 着色器代码和参数值，Qt3D 负责在渲染时把参数传递给 GPU。

```cpp
// 创建自定义材质
auto *material = new Qt3DRender::QMaterial();

// 顶点着色器
auto *shaderProgram = new Qt3DRender::QShaderProgram();
shaderProgram->setVertexShaderCode(
    Qt3DRender::QShaderProgram::loadSource(
        QUrl("qrc:/shaders/custom.vert")));
shaderProgram->setFragmentShaderCode(
    Qt3DRender::QShaderProgram::loadSource(
        QUrl("qrc:/shaders/custom.frag")));

// 添加参数（uniform 变量）
auto *colorParam = new Qt3DRender::QParameter(
    "u_color", QColor(0.2f, 0.6f, 1.0f));
auto *timeParam = new Qt3DRender::QParameter(
    "u_time", 0.0f);
material->addParameter(colorParam);
material->addParameter(timeParam);

// 渲染 Pass
auto *pass = new Qt3DRender::QRenderPass();
pass->setShaderProgram(shaderProgram);

auto *technique = new Qt3DRender::QTechnique();
technique->graphicsApiFilter()->setApi(
    Qt3DRender::QGraphicsApiFilter::OpenGL);
technique->graphicsApiFilter()->setMajorVersion(3);
technique->graphicsApiFilter()->setMinorVersion(0);
technique->addRenderPass(pass);

material->addTechnique(technique);
```

对应的简化 fragment shader：

```glsl
// custom.frag
uniform vec3 u_color;
uniform float u_time;
varying vec3 v_normal;

void main()
{
    float pulse = 0.5 + 0.5 * sin(u_time * 2.0);
    vec3 color = u_color * pulse;
    gl_FragColor = vec4(color, 1.0);
}
```

C++ 侧可以通过 `QParameter` 动态更新 uniform 值：

```cpp
// 动画循环中更新时间参数
timeParam->setValue(elapsedTime);
```

### 3.2 Framegraph——渲染管线配置

Qt3D 的 Framegraph 是一棵节点树，描述了渲染管线的完整配置——从视口设置到渲染目标到清除颜色到相机选择。你通过组合不同的 Framegraph 节点来构建复杂的渲染管线。

```cpp
// 基本渲染管线
auto *viewport = new Qt3DRender::QViewport(renderSettings);
auto *cameraSelector = new Qt3DRender::QCameraSelector(viewport);
cameraSelector->setCamera(camera);

auto *clearBuffers = new Qt3DRender::QClearBuffers(cameraSelector);
clearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
clearBuffers->setClearColor(Qt::black);

renderSettings->setActiveFrameGraph(viewport);
```

Framegraph 的强大之处在于你可以用分支节点实现多 Pass 渲染。比如先渲染场景到纹理（用于后处理），再渲染一个全屏四边形应用模糊效果：

```cpp
// Pass 1：渲染到纹理
auto *renderTarget = new Qt3DRender::QRenderTarget();
// ... 配置 render target ...

auto *pass1Selector = new Qt3DRender::QRenderTargetSelector(viewport);
pass1Selector->setTarget(renderTarget);
// ... 渲染场景 ...

// Pass 2：后处理全屏四边形
auto *pass2 = new Qt3DRender::QRenderPassSelector(viewport);
// ... 渲染后处理 quad ...
```

## 4. 踩坑预防

Qt3D 在 Qt 6 中的维护状态需要关注——它不是 Qt 的核心模块，更新节奏较慢。如果你需要高性能 3D 渲染，Qt 推荐使用 QtQuick3D（基于 Scene Graph + RHI 而不是 Qt3D 的 ECS 架构）。

## 5. 练习项目

练习项目：脉冲效果立方体。一个 3D 立方体使用自定义 GLSL 着色器，颜色随时间脉冲变化。通过 QParameter 动态更新 u_time。Framegraph 配置深色背景和抗锯齿。

## 6. 官方文档参考链接

[Qt 文档 · Qt 3D](https://doc.qt.io/qt-6/qt3d-index.html) -- Qt3D 模块总览

[Qt 文档 · QShaderProgram](https://doc.qt.io/qt-6/qshaderprogram.html) -- 着色器程序加载

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。自定义 Material 和 Framegraph 让你能完全控制 Qt3D 的渲染管线。
