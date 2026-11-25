---
title: "5.15 QtQuick3D 进阶：PBR 材质、环境光遮蔽、阴影"
description: "入门篇我们把 QtQuick3D 的基本 3D 场景跑通了——View3D + Model + PerspectiveCamera，显示一个带默认材质的 3D 模型。但入门篇用的只是最基础的 PrincipledMaterial 默认参数。QtQuick3D 支持完整的 PBR（Physically Based Rendering）材质管线，包括金属度、粗糙度、法线贴图、环境光遮蔽，以及方向光阴影。"
---

# 现代Qt开发教程（进阶篇）5.15——QtQuick3D 进阶：PBR 材质、环境光遮蔽、阴影

## 1. 前言

入门篇我们把 QtQuick3D 的基本 3D 场景跑通了——View3D + Model + PerspectiveCamera，显示一个带默认材质的 3D 模型。但入门篇用的只是最基础的 PrincipledMaterial 默认参数。QtQuick3D 支持完整的 PBR（Physically Based Rendering）材质管线，包括金属度、粗糙度、法线贴图、环境光遮蔽，以及方向光阴影。

PBR 是现代实时渲染的标准方法。它的核心理念是用物理参数（而不是经验参数）来描述材质——金属度（metalness）描述材质是金属还是电介质、粗糙度（roughness）描述表面的微观粗糙程度。这让不同光照条件下的材质表现更加真实和一致。

这篇我们把 PBR 材质的参数配置、环境光遮蔽贴图、以及阴影渲染这三个核心能力拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 QML + C++ 混合开发模式。本篇依赖 Qt6::Quick3D 模块。QtQuick3D 使用 Qt 的 RHI（Rendering Hardware Interface）抽象层，支持 OpenGL、Vulkan、Metal 和 Direct3D 后端。

## 3. 核心概念讲解

### 3.1 PrincipledMaterial——PBR 参数详解

`PrincipledMaterial` 是 QtQuick3D 的标准 PBR 材质。它的核心参数对应 PBR 管线的标准配置。

```qml
Model {
    source: "#Sphere"

    materials: PrincipledMaterial {
        baseColor: "#4488cc"
        metalness: 0.8
        roughness: 0.3

        // 法线贴图——给表面添加微观凹凸细节
        normalMap: Texture {
            source: "textures/normal.png"
        }

        // 金属度贴图——不同区域可以是金属或非金属
        metalnessMap: Texture {
            source: "textures/metalness.png"
        }

        // 粗糙度贴图——不同区域的粗糙程度
        roughnessMap: Texture {
            source: "textures/roughness.png"
        }

        // 自发光——材质本身发光
        emissiveMap: Texture {
            source: "textures/emissive.png"
        }
        emissiveColor: "#ffffff"
    }
}
```

关键参数的含义：`metalness: 0.0` 表示非金属（塑料、木材、布料），`metalness: 1.0` 表示纯金属（金、银、铝）。`roughness: 0.0` 表示完美镜面（反射清晰），`roughness: 1.0` 表示完全粗糙（漫反射）。真实世界中的材质很少是极端值——塑料约 0.05 金属度 + 0.4-0.6 粗糙度，拉丝不锈钢约 0.9 金属度 + 0.3 粗糙度。

### 3.2 环境光遮蔽（AO）

环境光遮蔽模拟场景中缝隙和角落由于几何遮挡导致的环境光照减少的效果。没有 AO 的场景看起来「扁平」，有 AO 的场景暗部细节更丰富。

```qml
Model {
    source: "models/room.obj"

    materials: PrincipledMaterial {
        baseColor: "#dddddd"
        roughness: 0.8
        metalness: 0.0

        // AO 贴图——0.0 表示完全遮蔽（黑色角落），1.0 表示完全暴露
        occlusionMap: Texture {
            source: "textures/ao.png"
        }
    }
}
```

AO 贴图通常在 3D 建模软件中烘焙生成——它预计算了每个像素被周围几何体遮挡的程度。QtQuick3D 在渲染时把 AO 因子乘到环境光照上，让遮挡区域变暗。

### 3.3 阴影——DirectionalLight + Shadow Maps

QtQuick3D 的 `DirectionalLight` 支持 `castsShadow: true`，启用后渲染管线会自动生成阴影贴图（Shadow Map）。

```qml
View3D {
    environment: SceneEnvironment {
        // 开启阴影需要设置合适的阴影质量
        shadowQuality: SceneEnvironment.ShadowQuality.High
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(-45, 45, 0)
        castsShadow: true
        shadowFactor: 80  // 阴影不透明度 0-100
        shadowMapQuality: Light.ShadowMapQuality.High
        shadowBias: 0.5
    }

    Model {
        source: "#Cube"
        castsShadow: true  // 这个物体投射阴影
        receivesShadow: true  // 这个物体接收阴影
        y: 1
    }

    // 地面
    Model {
        source: "#Rectangle"
        receivesShadow: true
        scale: Qt.vector3d(10, 10, 1)
        eulerRotation: Qt.vector3d(-90, 0, 0)
    }
}
```

`shadowBias` 解决阴影 acne（条纹伪影）问题。值太小会出现条纹，值太大会让阴影脱离物体底部。通常 0.5-1.0 是合理的范围。

## 4. 踩坑预防

阴影贴图的分辨率受 `shadowMapQuality` 控制。高质量意味着更大的纹理和更好的阴影边缘，但也消耗更多 GPU 显存。移动设备上建议用 Medium 或 Low。

## 5. 练习项目

练习项目：PBR 材质展示球。在一个 View3D 中放置 5 个球体，分别展示不同材质：塑料、金属、玻璃（透射）、橡胶、发光体。使用方向光 + 阴影。提供一个滑块控制粗糙度和金属度，实时切换材质效果。

## 6. 官方文档参考链接

[Qt 文档 · Qt Quick 3D](https://doc.qt.io/qt-6/qtquick3d-index.html) -- QtQuick3D 模块总览

[Qt 文档 · PrincipledMaterial](https://doc.qt.io/qt-6/qml-qtquick3d-principledmaterial.html) -- PBR 材质 QML 类型

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。PBR 材质、环境光遮蔽、阴影——这三个要素组合起来，你的 3D 场景就能达到现代游戏的视觉质量标准。
