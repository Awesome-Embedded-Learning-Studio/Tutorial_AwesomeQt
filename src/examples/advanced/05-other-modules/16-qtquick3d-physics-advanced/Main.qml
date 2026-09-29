// Main.qml — Qt Quick 3D physics simulation demo
//
// Demonstrates dynamic bodies (sphere, box) subject to gravity, a static
// ground plane, and impulse force application. Since QtQuick3D.Physics
// is not available on this system, the simulation is implemented using
// basic transforms, timers, and simple Euler integration.
//
// Key concepts shown:
//   - Gravity: constant downward acceleration each frame
//   - Ground collision: position clamped at ground level, velocity damped
//   - Impulse: instantaneous velocity change triggered by button press
//   - Static vs dynamic bodies: ground does not move, objects respond to forces

import QtQuick
import QtQuick3D

Window {
    id: root
    width: 1024
    height: 768
    visible: true
    title: "Qt Quick 3D — Physics Simulation"
    color: "#1a1a2e"

    // C++ backend providing gravity and force strength
    PhysicsConfig {
        id: physicsConfig

        onForceApplied: {
            // Apply upward + random lateral impulse to each dynamic body
            // Dividing by mass gives velocity change: dv = F / m
            var impulse = physicsConfig.forceStrength;

            // Sphere: mass = 2.0
            sphereBody.vy += impulse / 2.0;
            sphereBody.vx += (Math.random() - 0.5) * impulse * 0.3 / 2.0;

            // Box: mass = 5.0 (heavier, less response)
            boxBody.vy += impulse / 5.0;
            boxBody.vx += (Math.random() - 0.5) * impulse * 0.3 / 5.0;
        }
    }

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        // ---- Camera ----

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 500)
            eulerRotation: Qt.vector3d(-20, 0, 0)
        }

        // ---- Lighting ----

        DirectionalLight {
            position: Qt.vector3d(200, 300, 200)
            eulerRotation: Qt.vector3d(-45, 20, 0)
            brightness: 1.2
            castsShadow: true
            shadowFactor: 50
        }

        PointLight {
            position: Qt.vector3d(-150, 200, 150)
            brightness: 400
            color: "#ffeedd"
            quadraticFade: 0.002
        }

        // ---- Ground Plane (Static Collider) ----
        // Does not move — acts as the static floor for collision.

        Model {
            id: groundPlane
            position: Qt.vector3d(0, -5, 0)
            source: "#Rectangle"
            scale: Qt.vector3d(10, 10, 1)
            eulerRotation: Qt.vector3d(-90, 0, 0)

            materials: PrincipledMaterial {
                baseColor: "#4a4a5a"
                roughness: 0.85
                metalness: 0.1
            }
        }

        // ---- Sphere (Dynamic Body) ----
        // Mass = 2.0 kg, starts elevated, falls under gravity.

        Model {
            id: sphereModel
            position: Qt.vector3d(-100, 200, 0)
            source: "#Sphere"
            scale: Qt.vector3d(1.2, 1.2, 1.2)

            materials: PrincipledMaterial {
                id: sphereMaterial
                baseColor: "#e74c3c"
                roughness: 0.3
                metalness: 0.6
            }
        }

        // ---- Box (Dynamic Body) ----
        // Mass = 5.0 kg, starts elevated, falls under gravity.

        Model {
            id: boxModel
            position: Qt.vector3d(100, 300, 0)
            source: "#Cube"
            scale: Qt.vector3d(1.0, 1.0, 1.0)

            materials: PrincipledMaterial {
                id: boxMaterial
                baseColor: "#3498db"
                roughness: 0.5
                metalness: 0.2
            }
        }
    }

    // ---- Physics state objects ----
    // Each dynamic body has position, velocity, and mass.
    // The physics timer performs Euler integration each tick.

    QtObject {
        id: sphereBody

        // Position is synced to sphereModel in the physics timer
        property real x: -100
        property real y: 200
        property real vx: 0
        property real vy: 0
        property real mass: 2.0

        // Ground collision threshold (model radius ~ 30 units)
        property real radius: 30
    }

    QtObject {
        id: boxBody

        property real x: 100
        property real y: 300
        property real vx: 0
        property real vy: 0
        property real mass: 5.0

        // Half-height of the box model in scene units
        property real halfHeight: 25
    }

    // Ground Y position (matches groundPlane.position.y)
    readonly property real kGroundY: -5

    // ---- Physics Timer (Euler integration) ----
    // Runs at ~60 FPS. Each tick:
    //   1. Apply gravity to velocity: vy += gravity * dt
    //   2. Update position: y += vy * dt, x += vx * dt
    //   3. Ground collision: if y <= ground, clamp and bounce

    Timer {
        id: physicsTimer
        interval: 16   // ~60 FPS
        running: true
        repeat: true

        // Fixed timestep in seconds
        readonly property real dt: 0.016

        // Coefficient of restitution — energy retained after bounce
        readonly property real kRestitution: 0.5

        // Lateral friction — reduces horizontal velocity each frame
        readonly property real kFriction: 0.98

        onTriggered: {
            // --- Sphere physics ---
            // Gravity: accelerate downward
            // physicsConfig.gravity is negative (e.g. -9.81), but we
            // treat positive Y as up in the scene, so negate it
            sphereBody.vy += (-physicsConfig.gravity) * dt;

            // Air resistance — gentle damping proportional to velocity
            sphereBody.vy *= 0.999;
            sphereBody.vx *= kFriction;

            // Integrate position
            sphereBody.y += sphereBody.vy * dt;
            sphereBody.x += sphereBody.vx * dt;

            // Ground collision: clamp and bounce
            var sphereGround = kGroundY + sphereBody.radius;
            if (sphereBody.y <= sphereGround) {
                sphereBody.y = sphereGround;
                // Reverse velocity and apply restitution
                sphereBody.vy = -sphereBody.vy * kRestitution;
                // Kill tiny bounces to avoid jitter
                if (Math.abs(sphereBody.vy) < 1.0) {
                    sphereBody.vy = 0;
                }
            }

            // Wall bounds — keep objects in view
            if (sphereBody.x < -250) { sphereBody.x = -250; sphereBody.vx = -sphereBody.vx * 0.5; }
            if (sphereBody.x > 250)  { sphereBody.x = 250;  sphereBody.vx = -sphereBody.vx * 0.5; }

            // --- Box physics ---
            boxBody.vy += (-physicsConfig.gravity) * dt;
            boxBody.vy *= 0.999;
            boxBody.vx *= kFriction;

            boxBody.y += boxBody.vy * dt;
            boxBody.x += boxBody.vx * dt;

            var boxGround = kGroundY + boxBody.halfHeight;
            if (boxBody.y <= boxGround) {
                boxBody.y = boxGround;
                boxBody.vy = -boxBody.vy * kRestitution;
                if (Math.abs(boxBody.vy) < 1.0) {
                    boxBody.vy = 0;
                }
            }

            if (boxBody.x < -250) { boxBody.x = -250; boxBody.vx = -boxBody.vx * 0.5; }
            if (boxBody.x > 250)  { boxBody.x = 250;  boxBody.vx = -boxBody.vx * 0.5; }

            // --- Sync positions to 3D models ---
            sphereModel.position = Qt.vector3d(sphereBody.x, sphereBody.y, 0);
            boxModel.position = Qt.vector3d(boxBody.x, boxBody.y, 0);

            // Rotate box proportionally to horizontal velocity for visual feedback
            boxModel.eulerRotation = Qt.vector3d(
                boxModel.eulerRotation.x + boxBody.vx * dt * 2,
                boxModel.eulerRotation.y,
                boxModel.eulerRotation.z
            );
        }
    }

    // ---- Drop Timer: periodically reset and drop objects ----
    // Resets bodies to elevated positions with zero velocity so the
    // user can observe free-fall and bouncing repeatedly.

    Timer {
        id: dropTimer
        interval: 8000    // Reset every 8 seconds
        running: true
        repeat: true

        onTriggered: {
            // Reset sphere
            sphereBody.x = -100;
            sphereBody.y = 200 + Math.random() * 100;
            sphereBody.vx = 0;
            sphereBody.vy = 0;

            // Reset box
            boxBody.x = 100;
            boxBody.y = 300 + Math.random() * 100;
            boxBody.vx = 0;
            boxBody.vy = 0;
        }
    }

    // ---- UI Controls ----

    Row {
        id: controls
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 20
        spacing: 12

        Button {
            text: "Apply Impulse"
            onClicked: {
                physicsConfig.applyForce();
            }
        }

        Button {
            text: "Reset Scene"
            onClicked: {
                // Immediately reset bodies without waiting for dropTimer
                sphereBody.x = -100;
                sphereBody.y = 250;
                sphereBody.vx = 0;
                sphereBody.vy = 0;

                boxBody.x = 100;
                boxBody.y = 350;
                boxBody.vx = 0;
                boxBody.vy = 0;
            }
        }
    }

    // ---- HUD overlay showing physics state ----

    Column {
        id: hud
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        spacing: 4

        Text {
            color: "#cccccc"
            font.pixelSize: 13
            font.family: "monospace"
            text: "Gravity: " + physicsConfig.gravity.toFixed(2) + " m/s^2"
                + "  Force: " + physicsConfig.forceStrength.toFixed(1) + " N"
        }

        Text {
            color: "#e74c3c"
            font.pixelSize: 13
            font.family: "monospace"
            text: "Sphere  y=" + sphereBody.y.toFixed(1)
                + "  vy=" + sphereBody.vy.toFixed(1)
                + "  mass=" + sphereBody.mass.toFixed(1)
        }

        Text {
            color: "#3498db"
            font.pixelSize: 13
            font.family: "monospace"
            text: "Box     y=" + boxBody.y.toFixed(1)
                + "  vy=" + boxBody.vy.toFixed(1)
                + "  mass=" + boxBody.mass.toFixed(1)
        }
    }
}
