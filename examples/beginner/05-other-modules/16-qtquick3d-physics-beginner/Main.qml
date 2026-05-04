import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Physics

Window {
    width: 1000
    height: 700
    visible: true
    title: "QtQuick3D Physics 物理模拟"

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
        }

        // 物理世界——配置重力加速度
        PhysicsWorld {
            id: physicsWorld
            running: true
            gravity: Qt.vector3d(0, -500, 0)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 300, 800)
            eulerRotation: Qt.vector3d(-20, 0, 0)
            fieldOfView: 50
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-60, 30, 0)
            brightness: 1.0
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-30, -45, 0)
            brightness: 0.3
        }

        // 地面——静态刚体
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(100, 100, 1)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            materials: PrincipledMaterial {
                baseColor: "#505050"
                roughness: 0.85
            }
            StaticRigidBody {
                restitution: 0.3
                friction: 0.6
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(10000, 100, 10000)
                }
            }
        }

        // 左墙——静态刚体
        Model {
            source: "#Cube"
            position: Qt.vector3d(-500, 250, 0)
            scale: Qt.vector3d(1, 5, 10)
            materials: PrincipledMaterial {
                baseColor: "#404060"
                roughness: 0.7
            }
            StaticRigidBody {
                restitution: 0.2
                friction: 0.5
                collisionShapes: BoxShape {}
            }
        }

        // 右墙——静态刚体
        Model {
            source: "#Cube"
            position: Qt.vector3d(500, 250, 0)
            scale: Qt.vector3d(1, 5, 10)
            materials: PrincipledMaterial {
                baseColor: "#404060"
                roughness: 0.7
            }
            StaticRigidBody {
                restitution: 0.2
                friction: 0.5
                collisionShapes: BoxShape {}
            }
        }

        // 动态球体——从高处掉落
        Model {
            id: sphereModel
            source: "#Sphere"
            position: Qt.vector3d(-100, 500, 0)
            scale: Qt.vector3d(50, 50, 50)
            materials: PrincipledMaterial {
                id: sphereMaterial
                baseColor: "#4488ff"
                metalness: 0.1
                roughness: 0.3
            }
            DynamicRigidBody {
                id: sphereBody
                mass: 1.0
                restitution: 0.75
                friction: 0.4
                collisionShapes: SphereShape {}
            }
        }

        // 动态立方体——从另一侧掉落
        Model {
            id: cubeModel
            source: "#Cube"
            position: Qt.vector3d(100, 600, 50)
            scale: Qt.vector3d(50, 50, 50)
            materials: PrincipledMaterial {
                id: cubeMaterial
                baseColor: "#e68a30"
                metalness: 0.3
                roughness: 0.5
            }
            DynamicRigidBody {
                id: cubeBody
                mass: 2.0
                restitution: 0.4
                friction: 0.5
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(50, 50, 50)
                }
            }
        }

        // 动态胶囊体——斜向抛出
        Model {
            id: capsuleModel
            source: "#Cylinder"
            position: Qt.vector3d(0, 400, -100)
            scale: Qt.vector3d(40, 80, 40)
            materials: PrincipledMaterial {
                id: capsuleMaterial
                baseColor: "#44cc66"
                metalness: 0.0
                roughness: 0.6
            }
            DynamicRigidBody {
                id: capsuleBody
                mass: 1.5
                restitution: 0.6
                friction: 0.7
                collisionShapes: CapsuleShape {
                    diameter: 40
                    height: 80
                }
                // 初始线速度，模拟斜向抛出
                linearVelocity: Qt.vector3d(80, 200, 30)
            }
        }
    }

    // 2D 控制面板
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 15
        width: 220
        height: 180
        color: "#c01a1a2e"
        radius: 12
        border.color: "#ffffff40"
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 10

            Label {
                text: "物理控制面板"
                font.bold: true
                font.pixelSize: 16
                color: "white"
            }

            Button {
                text: "重置所有物体"
                onClicked: {
                    sphereModel.position = Qt.vector3d(-100, 500, 0);
                    sphereBody.linearVelocity = Qt.vector3d(0, 0, 0);
                    sphereBody.angularVelocity = Qt.vector3d(0, 0, 0);

                    cubeModel.position = Qt.vector3d(100, 600, 50);
                    cubeBody.linearVelocity = Qt.vector3d(0, 0, 0);
                    cubeBody.angularVelocity = Qt.vector3d(0, 0, 0);

                    capsuleModel.position = Qt.vector3d(0, 400, -100);
                    capsuleBody.linearVelocity = Qt.vector3d(80, 200, 30);
                    capsuleBody.angularVelocity = Qt.vector3d(0, 0, 0);
                }
            }

            Button {
                text: physicsWorld.running ? "暂停模拟" : "继续模拟"
                onClicked: physicsWorld.running = !physicsWorld.running
            }

            Label {
                text: "重力: " + physicsWorld.gravity.y.toFixed(0) + " cm/s\u00B2"
                color: "#aaa"
                font.pixelSize: 12
            }
        }
    }
}
