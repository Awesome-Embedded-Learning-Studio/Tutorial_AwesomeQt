// Main.qml — Qt Quick 3D scene demonstrating PBR material configuration
//
// Displays two models (sphere and cube) with independently configured
// PrincipledMaterial instances. A timer animates the sphere's metalness
// and roughness to showcase reactive C++ ↔ QML property binding.

import QtQuick
import QtQuick3D

Window {
    id: root
    width: 1024
    height: 768
    visible: true
    title: "Qt Quick 3D — PBR Material Demo"
    color: "#1a1a2e"

    // C++ backend instance providing PBR property defaults
    SceneConfig {
        id: sceneConfig
    }

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
            // Enable anti-aliasing for smoother model edges
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        // ---- Camera ----

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 120, 400)
            eulerRotation: Qt.vector3d(-15, 0, 0)
        }

        // ---- Lighting ----

        DirectionalLight {
            id: dirLight
            position: Qt.vector3d(200, 300, 200)
            eulerRotation: Qt.vector3d(-45, 20, 0)
            brightness: 1.2
            // Cast shadows for depth perception
            castsShadow: true
            shadowFactor: 50
        }

        PointLight {
            id: pointLight
            position: Qt.vector3d(-150, 200, 150)
            brightness: 400
            // Color tint adds visual interest
            color: "#ffeedd"
            quadraticFade: 0.002
        }

        // ---- Sphere Model (animated PBR via C++ backend) ----

        Model {
            id: sphere
            position: Qt.vector3d(-120, 0, 0)
            source: "#Sphere"
            scale: Qt.vector3d(1.5, 1.5, 1.5)

            materials: PrincipledMaterial {
                id: sphereMaterial
                // Bind directly to SceneConfig properties — changes propagate reactively
                metalness: sceneConfig.metalness
                roughness: sceneConfig.roughness
                baseColor: sceneConfig.baseColor
            }
        }

        // ---- Cube Model (static PBR for visual contrast) ----

        Model {
            id: cube
            position: Qt.vector3d(120, 0, 0)
            source: "#Cube"
            scale: Qt.vector3d(1.5, 1.5, 1.5)

            materials: PrincipledMaterial {
                id: cubeMaterial
                // Matte ceramic look — low metalness, moderate roughness
                metalness: 0.0
                roughness: 0.6
                baseColor: "#e8d5b7"
            }
        }
    }

    // ---- Timer: animate sphere PBR parameters ----
    // Oscillates metalness and roughness so the user sees live
    // material changes driven by C++ property bindings.

    Timer {
        id: animateTimer
        interval: 50        // ~20 FPS update rate — sufficient for smooth PBR changes
        running: true
        repeat: true

        property real t: 0.0

        onTriggered: {
            t += 0.02;
            // Sinusoidal oscillation keeps values in [0.0, 1.0]
            sceneConfig.metalness = 0.5 + 0.5 * Math.sin(t);
            sceneConfig.roughness = 0.5 + 0.5 * Math.cos(t * 0.7);

            // Subtle color shift to demonstrate QColor binding
            var r = Math.round(70 + 50 * Math.sin(t * 0.5));
            var g = Math.round(130 + 40 * Math.cos(t * 0.3));
            var b = Math.round(180 + 30 * Math.sin(t * 0.8));
            sceneConfig.baseColor = Qt.rgba(r / 255, g / 255, b / 255, 1.0);

            // Slowly rotate the cube for visual feedback
            cube.eulerRotation = Qt.vector3d(0, t * 30, 0);
        }
    }

    // ---- HUD overlay showing current PBR values ----

    Text {
        id: hud
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        color: "#cccccc"
        font.pixelSize: 14
        font.family: "monospace"
        text: "Metalness: " + sceneConfig.metalness.toFixed(2)
            + "  Roughness: " + sceneConfig.roughness.toFixed(2)
    }
}
