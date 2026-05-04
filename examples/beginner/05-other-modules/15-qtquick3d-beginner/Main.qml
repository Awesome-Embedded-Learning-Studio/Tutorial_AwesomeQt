/**
 * QtQuick3D 基础场景 - QML 主文件
 *
 * 本文件演示 QtQuick3D 的核心组件使用：
 * 1. View3D 创建 3D 渲染视口
 * 2. PerspectiveCamera 透视相机
 * 3. DirectionalLight 方向光源
 * 4. Model + PrincipledMaterial PBR 几何体与材质
 * 5. Qt Quick 2D 控件与 3D 场景混合布局
 */

import QtQuick
import QtQuick.Controls
import QtQuick3D

Window {
    id: root
    width: 1000
    height: 700
    visible: true
    title: "QtQuick3D 基础场景 - 球体 / 立方体 / 2D 混合"
    color: "#1a1a2e"

    // ========================================
    // 3D 视口
    // ========================================

    View3D {
        id: view3d
        anchors.fill: parent

        // 场景环境：深色背景
        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
        }

        // 透视相机
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 10, 25)
            eulerRotation: Qt.vector3d(-20, 0, 0)
            fieldOfView: 45
            clipNear: 0.1
            clipFar: 1000
        }

        // ========================================
        // 光源
        // ========================================

        // 主方向光：从右上方照射
        DirectionalLight {
            eulerRotation: Qt.vector3d(-60, 45, 0)
            brightness: 1.0
            castsShadow: true
        }

        // 补光：从左侧补光，减弱阴影
        DirectionalLight {
            eulerRotation: Qt.vector3d(-30, -45, 0)
            brightness: 0.3
            color: "#d0d0e8"
        }

        // ========================================
        // 球体（蓝色，偏左）
        // ========================================

        Model {
            id: sphereModel
            source: "#Sphere"
            position: Qt.vector3d(-4, 1.5, 0)
            scale: Qt.vector3d(1.5, 1.5, 1.5)

            materials: PrincipledMaterial {
                id: sphereMaterial
                baseColor: "#4080ff"
                metalness: 0.1
                roughness: 0.4
            }

            // 球体绕 Y 轴旋转动画
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                NumberAnimation {
                    from: 0
                    to: 360
                    duration: 6000
                    easing.type: Easing.Linear
                }
            }
        }

        // ========================================
        // 立方体（橙色，偏右）
        // ========================================

        Model {
            id: cubeModel
            source: "#Cube"
            position: Qt.vector3d(4, 1, 0)
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation: Qt.vector3d(15, 0, 0)

            materials: PrincipledMaterial {
                id: cubeMaterial
                baseColor: "#e68a30"
                metalness: 0.5
                roughness: 0.3
            }

            // 立方体绕 Y 轴旋转动画（比球体慢一点）
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                NumberAnimation {
                    from: 0
                    to: 360
                    duration: 4000
                    easing.type: Easing.Linear
                }
            }
        }

        // ========================================
        // 地面（灰色平面）
        // ========================================

        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(30, 30, 1)
            eulerRotation: Qt.vector3d(-90, 0, 0)

            materials: PrincipledMaterial {
                baseColor: "#606060"
                roughness: 0.85
                metalness: 0.0
            }
        }

        // 3D 场景标题文字（叠加在 3D 视图内）
        Text {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 20
            text: "QtQuick3D 基础场景"
            font.pixelSize: 22
            font.bold: true
            color: "white"
            style: Text.Outline
            styleColor: "black"
        }
    }

    // ========================================
    // 2D 控制面板（浮动在 3D 场景上方）
    // ========================================

    Rectangle {
        id: controlPanel
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 15
        width: 230
        height: controlColumn.height + 30
        color: "#c01a1a2e"
        radius: 12
        border.color: "#ffffff40"
        border.width: 1

        Column {
            id: controlColumn
            anchors.fill: parent
            anchors.margins: 15
            spacing: 10

            // 面板标题
            Label {
                text: "控制面板"
                font.bold: true
                font.pixelSize: 16
                color: "white"
            }

            // 分隔线
            Rectangle {
                width: parent.width
                height: 1
                color: "#ffffff30"
            }

            // 球体颜色预设
            Label {
                text: "球体颜色"
                font.pixelSize: 13
                color: "#cccccc"
            }

            Row {
                spacing: 8

                Button {
                    text: "蓝"
                    font.pixelSize: 12
                    onClicked: sphereMaterial.baseColor = "#4080ff"
                }
                Button {
                    text: "红"
                    font.pixelSize: 12
                    onClicked: sphereMaterial.baseColor = "#ff4444"
                }
                Button {
                    text: "绿"
                    font.pixelSize: 12
                    onClicked: sphereMaterial.baseColor = "#44cc44"
                }
                Button {
                    text: "紫"
                    font.pixelSize: 12
                    onClicked: sphereMaterial.baseColor = "#aa44ff"
                }
            }

            // 分隔线
            Rectangle {
                width: parent.width
                height: 1
                color: "#ffffff30"
            }

            // 立方体金属度滑块
            Label {
                text: "立方体金属度: " + metalnessSlider.value.toFixed(2)
                font.pixelSize: 13
                color: "#cccccc"
            }

            Slider {
                id: metalnessSlider
                from: 0.0
                to: 1.0
                value: 0.5
                stepSize: 0.01
                width: parent.width

                onValueChanged: {
                    cubeMaterial.metalness = value
                }
            }

            // 重置按钮
            Button {
                text: "重置所有"
                font.pixelSize: 12
                width: parent.width
                onClicked: {
                    sphereMaterial.baseColor = "#4080ff"
                    cubeMaterial.baseColor = "#e68a30"
                    cubeMaterial.metalness = 0.5
                    metalnessSlider.value = 0.5
                }
            }
        }
    }

    // 操作提示（底部）
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 15
        text: "通过右侧控制面板切换颜色和材质参数"
        font.pixelSize: 13
        color: "#888888"
    }
}
