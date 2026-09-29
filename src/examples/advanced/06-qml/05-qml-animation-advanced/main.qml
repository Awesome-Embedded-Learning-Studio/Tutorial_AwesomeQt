// main.qml -- QML Animation Advanced Demo
// Demonstrates: PathAnimation, SmoothedAnimation,
//               SpringAnimation, OpacityAnimator

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "QML Animation Advanced"

    ScrollView {
        anchors.fill: parent
        clip: true

        Column {
            width: Math.max(window.width, 760)
            spacing: 24

            // ====== Title ======
            Label {
                text: "QML Animation Advanced"
                font.pixelSize: 24
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                topPadding: 16
            }

            // ================================================================
            // Demo 1: PathAnimation
            // Rectangle follows a path composed of PathLine + PathArc.
            // PathAnimation moves the target along a geometric path,
            // which is more flexible than animating x/y independently.
            // ================================================================
            GroupBox {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                title: "1. PathAnimation"

                Column {
                    spacing: 8
                    width: parent.width

                    Label {
                        text: "Rectangle follows PathLine + PathArc. Press the button to start."
                        font.pixelSize: 12
                        color: "#888"
                    }

                    Rectangle {
                        width: parent.width
                        height: 160
                        color: "#f0f0f0"
                        radius: 8
                        clip: true

                        Rectangle {
                            id: pathRect
                            width: 30
                            height: 30
                            radius: 15
                            color: "#3498db"
                            // PathAnimation controls x and y, so set initial pos
                            x: 20
                            y: 20

                            Text {
                                anchors.centerIn: parent
                                text: "P"
                                color: "white"
                                font.bold: true
                                font.pixelSize: 12
                            }
                        }

                        // PathAnimation moves pathRect along the defined path.
                        // orientation: AnglePathAuto rotates the item to match
                        // the path tangent direction.
                        PathAnimation {
                            id: pathAnim
                            target: pathRect
                            duration: 3000
                            loops: 1
                            orientation: PathAnimation.AnglePathAuto
                            orientationEntryDuration: 200

                            path: Path {
                                // Start at top-left
                                startX: 20
                                startY: 20

                                // Straight line to top-right corner
                                PathLine {
                                    x: pathRect.parent.width - 50
                                    y: 20
                                }

                                // Arc down to bottom-right
                                PathArc {
                                    x: pathRect.parent.width - 50
                                    y: pathRect.parent.height - 50
                                    radiusX: 80
                                    radiusY: 80
                                }

                                // Straight line back to bottom-left
                                PathLine {
                                    x: 20
                                    y: pathRect.parent.height - 50
                                }

                                // Arc back up to start
                                PathArc {
                                    x: 20
                                    y: 20
                                    radiusX: 80
                                    radiusY: 80
                                }
                            }
                        }
                    }

                    Button {
                        text: pathAnim.running ? "Running..." : "Start PathAnimation"
                        onClicked: {
                            pathAnim.stop()
                            pathAnim.start()
                        }
                    }
                }
            }

            // ================================================================
            // Demo 2: SmoothedAnimation
            // SmoothedAnimation smoothly interpolates toward a target value
            // with configurable velocity and reversal behavior. Unlike a
            // plain NumberAnimation, it handles mid-animation target changes
            // gracefully -- the item decelerates, reverses if needed, and
            // heads toward the new target.
            // ================================================================
            GroupBox {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                title: "2. SmoothedAnimation"

                Column {
                    spacing: 8
                    width: parent.width

                    Label {
                        text: "Green rect follows the red target. Press Move to randomize target."
                        font.pixelSize: 12
                        color: "#888"
                    }

                    Rectangle {
                        width: parent.width
                        height: 120
                        color: "#f0f0f0"
                        radius: 8
                        clip: true

                        // The "target" -- red dot that jumps to random positions
                        Rectangle {
                            id: smoothTarget
                            width: 16
                            height: 16
                            radius: 8
                            color: "#e74c3c"
                            x: 20
                            y: 52
                        }

                        // The "follower" -- uses SmoothedAnimation on x and y
                        // to smoothly chase the target position.
                        Rectangle {
                            id: smoothFollower
                            width: 30
                            height: 30
                            radius: 15
                            color: "#2ecc71"

                            Text {
                                anchors.centerIn: parent
                                text: "S"
                                color: "white"
                                font.bold: true
                                font.pixelSize: 12
                            }

                            // SmoothedAnimation on x: velocity-limited chase
                            Behavior on x {
                                SmoothedAnimation {
                                    velocity: 200
                                    reversingMode: SmoothedAnimation.Sync
                                }
                            }

                            // SmoothedAnimation on y: same configuration
                            Behavior on y {
                                SmoothedAnimation {
                                    velocity: 200
                                    reversingMode: SmoothedAnimation.Sync
                                }
                            }

                            // Bind follower to target center
                            x: smoothTarget.x - 7
                            y: smoothTarget.y - 7
                        }
                    }

                    Button {
                        text: "Move Target"
                        onClicked: {
                            // Jump the target to a random position within the
                            // play area. The follower will smoothly chase it.
                            var playArea = smoothTarget.parent
                            smoothTarget.x = Math.random() * (playArea.width - 40) + 20
                            smoothTarget.y = Math.random() * (playArea.height - 40) + 20
                        }
                    }
                }
            }

            // ================================================================
            // Demo 3: SpringAnimation
            // SpringAnimation simulates spring physics with configurable
            // spring constant and damping. The item overshoots and oscillates
            // around the target, creating a natural "bouncy" feel.
            // spring: stiffness of the spring (higher = faster snap)
            // damping: how quickly oscillation dies out (lower = more bounce)
            // epsilon: convergence threshold to stop the animation
            // ================================================================
            GroupBox {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                title: "3. SpringAnimation"

                Column {
                    spacing: 8
                    width: parent.width

                    Label {
                        text: "Spring: 2.0, Damping: 0.2 -- notice the overshoot and oscillation."
                        font.pixelSize: 12
                        color: "#888"
                    }

                    Rectangle {
                        width: parent.width
                        height: 120
                        color: "#f0f0f0"
                        radius: 8
                        clip: true

                        Rectangle {
                            id: springRect
                            width: 40
                            height: 40
                            radius: 20
                            color: "#9b59b6"
                            x: 20
                            y: 40

                            Text {
                                anchors.centerIn: parent
                                text: "Spr"
                                color: "white"
                                font.bold: true
                                font.pixelSize: 11
                            }

                            // SpringAnimation on x: low damping = long oscillation
                            Behavior on x {
                                SpringAnimation {
                                    spring: 2.0
                                    damping: 0.2
                                    epsilon: 0.005
                                }
                            }

                            // SpringAnimation on y: same params
                            Behavior on y {
                                SpringAnimation {
                                    spring: 2.0
                                    damping: 0.2
                                    epsilon: 0.005
                                }
                            }
                        }
                    }

                    Button {
                        text: "Trigger Spring"
                        onClicked: {
                            // Jump to a random position. SpringAnimation
                            // in Behavior will animate the transition.
                            var area = springRect.parent
                            springRect.x = Math.random() * (area.width - 60) + 10
                            springRect.y = Math.random() * (area.height - 60) + 10
                        }
                    }
                }
            }

            // ================================================================
            // Demo 4: OpacityAnimator (Animator type)
            // Animator types (OpacityAnimator, ScaleAnimator, etc.) run on
            // Qt's render thread instead of the UI thread. This keeps
            // animations smooth even when the UI thread is busy.
            // They are used inside Transition blocks and cannot be used
            // standalone like NumberAnimation.
            // ================================================================
            GroupBox {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                title: "4. OpacityAnimator (Render-Thread Animator)"

                Column {
                    spacing: 8
                    width: parent.width

                    Label {
                        text: "OpacityAnimator runs on the render thread for smoother results."
                        font.pixelSize: 12
                        color: "#888"
                    }

                    Rectangle {
                        width: parent.width
                        height: 100
                        color: "#f0f0f0"
                        radius: 8
                        clip: true

                        Rectangle {
                            id: fadeRect
                            width: 120
                            height: 60
                            radius: 12
                            color: "#e67e22"
                            anchors.centerIn: parent
                            opacity: 1.0

                            // Two states: visible and faded
                            states: [
                                State {
                                    name: "visible"
                                    PropertyChanges {
                                        target: fadeRect
                                        opacity: 1.0
                                    }
                                },
                                State {
                                    name: "faded"
                                    PropertyChanges {
                                        target: fadeRect
                                        opacity: 0.1
                                    }
                                }
                            ]

                            // OpacityAnimator inside Transition: runs on the
                            // render thread so the fade stays smooth even if
                            // the UI thread is under load.
                            transitions: [
                                Transition {
                                    to: "visible"
                                    OpacityAnimator {
                                        target: fadeRect
                                        duration: 800
                                        easing.type: Easing.InOutQuad
                                    }
                                },
                                Transition {
                                    to: "faded"
                                    OpacityAnimator {
                                        target: fadeRect
                                        duration: 800
                                        easing.type: Easing.InOutQuad
                                    }
                                }
                            ]

                            Text {
                                anchors.centerIn: parent
                                text: "Fade Me"
                                color: "white"
                                font.bold: true
                                font.pixelSize: 14
                            }
                        }
                    }

                    Button {
                        text: fadeRect.state === "faded" ? "Fade In" : "Fade Out"
                        onClicked: {
                            fadeRect.state = (fadeRect.state === "faded")
                                             ? "visible" : "faded"
                        }
                    }
                }
            }

            // Bottom padding
            Item {
                width: 1
                height: 24
            }
        }
    }
}
