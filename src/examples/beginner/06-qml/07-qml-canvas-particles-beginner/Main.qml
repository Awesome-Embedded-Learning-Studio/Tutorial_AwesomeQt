// Main.qml — QML Canvas 绘图与粒子系统综合演示

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Particles

ApplicationWindow {
    id: window
    width: 800
    height: 750
    visible: true
    title: "QML Canvas & Particles Demo"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        Label {
            text: "Canvas & Particles Demo"
            font.pixelSize: 24
            font.bold: true
        }

        // ====== 1. Canvas 涂鸦板 ======
        GroupBox {
            Layout.fillWidth: true
            title: "Canvas - Drawing Pad"

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label { text: "Color:" }
                    Repeater {
                        model: ["#e74c3c", "#3498db", "#2ecc71", "#f39c12", "#9b59b6", "#1a1a2e"]

                        Rectangle {
                            width: 28
                            height: 28
                            radius: 14
                            color: modelData
                            border.color: drawCanvas.penColor === modelData ? "#333" : "transparent"
                            border.width: 2

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: drawCanvas.penColor = modelData
                            }
                        }
                    }

                    Label { text: "Size:" }
                    Slider {
                        id: penSizeSlider
                        from: 2
                        to: 20
                        value: 4
                        stepSize: 1
                        Layout.preferredWidth: 100
                    }

                    Button {
                        text: "Clear"
                        onClicked: {
                            drawCanvas.clearCanvas()
                        }
                    }
                }

                Canvas {
                    id: drawCanvas
                    Layout.fillWidth: true
                    height: 200

                    property string penColor: "#1a1a2e"
                    property real penSize: penSizeSlider.value
                    property point lastPoint

                    function clearCanvas()
                    {
                        var ctx = getContext('2d')
                        ctx.fillStyle = '#ffffff'
                        ctx.fillRect(0, 0, width, height)
                        requestPaint()
                    }

                    onPaint: {
                        var ctx = getContext('2d')

                        if (drawMouse.pressed) {
                            ctx.strokeStyle = penColor
                            ctx.lineWidth = penSize
                            ctx.lineCap = 'round'
                            ctx.lineJoin = 'round'
                            ctx.beginPath()
                            ctx.moveTo(lastPoint.x, lastPoint.y)
                            ctx.lineTo(drawMouse.mouseX, drawMouse.mouseY)
                            ctx.stroke()
                            lastPoint = Qt.point(drawMouse.mouseX, drawMouse.mouseY)
                        }
                    }

                    Component.onCompleted: clearCanvas()

                    MouseArea {
                        id: drawMouse
                        anchors.fill: parent

                        onPressed: function(mouse) {
                            drawCanvas.lastPoint = Qt.point(mouse.x, mouse.y)
                        }
                        onPositionChanged: function(mouse) {
                            if (pressed) {
                                drawCanvas.requestPaint()
                            }
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "transparent"
                        border.color: "#ddd"
                        radius: 4
                    }
                }
            }
        }

        // ====== 2. Canvas 轨道动画 ======
        GroupBox {
            Layout.fillWidth: true
            title: "Canvas - Orbital Animation"

            Canvas {
                id: orbitCanvas
                Layout.fillWidth: true
                height: 200

                property real time: 0

                onPaint: {
                    var ctx = getContext('2d')
                    ctx.clearRect(0, 0, width, height)

                    // 深色背景
                    ctx.fillStyle = '#0d1b2a'
                    ctx.fillRect(0, 0, width, height)

                    var cx = width / 2
                    var cy = height / 2

                    // 画轨道环
                    for (var ring = 0; ring < 3; ring++) {
                        var orbitRadius = 30 + ring * 30
                        ctx.beginPath()
                        ctx.arc(cx, cy, orbitRadius, 0, Math.PI * 2)
                        ctx.strokeStyle = 'rgba(255, 255, 255, 0.08)'
                        ctx.lineWidth = 1
                        ctx.stroke()

                        // 画轨道上的球
                        var speed = (3 - ring) * 1.5
                        var angle = time * speed + ring * 2.0
                        var bx = cx + Math.cos(angle) * orbitRadius
                        var by = cy + Math.sin(angle) * orbitRadius
                        var ballRadius = 8 - ring * 1.5

                        // 球的光晕
                        var glow = ctx.createRadialGradient(bx, by, 0, bx, by, ballRadius * 3)
                        var hue = ring / 3
                        glow.addColorStop(0, Qt.hsla(hue, 0.9, 0.6, 0.8))
                        glow.addColorStop(1, 'transparent')
                        ctx.beginPath()
                        ctx.arc(bx, by, ballRadius * 3, 0, Math.PI * 2)
                        ctx.fillStyle = glow
                        ctx.fill()

                        // 球体
                        ctx.beginPath()
                        ctx.arc(bx, by, ballRadius, 0, Math.PI * 2)
                        ctx.fillStyle = Qt.hsla(hue, 0.9, 0.6, 1.0)
                        ctx.fill()
                    }

                    // 中心太阳
                    var sunGlow = ctx.createRadialGradient(cx, cy, 0, cx, cy, 25)
                    sunGlow.addColorStop(0, '#f1c40f')
                    sunGlow.addColorStop(0.5, '#e67e22')
                    sunGlow.addColorStop(1, 'transparent')
                    ctx.beginPath()
                    ctx.arc(cx, cy, 25, 0, Math.PI * 2)
                    ctx.fillStyle = sunGlow
                    ctx.fill()

                    ctx.beginPath()
                    ctx.arc(cx, cy, 10, 0, Math.PI * 2)
                    ctx.fillStyle = '#f39c12'
                    ctx.fill()
                }

                Timer {
                    interval: 16
                    running: true
                    repeat: true
                    onTriggered: {
                        orbitCanvas.time += 0.016
                        orbitCanvas.requestPaint()
                    }
                }
            }
        }

        // ====== 3. 粒子系统 ======
        GroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: "ParticleSystem - Fireworks"

            Item {
                anchors.fill: parent
                clip: true

                ParticleSystem {
                    id: fireworksSystem
                }

                // 火花发射器
                Emitter {
                    id: sparkEmitter
                    system: fireworksSystem
                    x: parent.width / 2
                    y: parent.height * 0.3

                    emitRate: 120
                    maximumEmitted: 300
                    lifeSpan: 2000
                    lifeSpanVariation: 800
                    size: 8
                    sizeVariation: 4

                    velocity: AngleDirection {
                        angle: 270
                        angleVariation: 30
                        magnitude: 80
                        magnitudeVariation: 40
                    }

                    acceleration: AngleDirection {
                        angle: 90
                        magnitude: 40
                    }

                    Timer {
                        interval: 3000
                        running: true
                        repeat: true
                        onTriggered: {
                            sparkEmitter.x = Math.random() * (parent.width - 100) + 50
                            sparkEmitter.y = Math.random() * (parent.height * 0.3) + parent.height * 0.1
                            sparkEmitter.pulse(150)
                        }
                    }
                }

                // 环境粒子
                Emitter {
                    system: fireworksSystem
                    anchors.fill: parent
                    emitRate: 30
                    lifeSpan: 4000
                    size: 3
                    sizeVariation: 2

                    velocity: AngleDirection {
                        angle: 90
                        angleVariation: 20
                        magnitude: 15
                    }
                }

                ImageParticle {
                    system: fireworksSystem
                    source: "qrc:///particleresources/glowdot.png"
                    color: "#f39c12"
                    colorVariation: 0.6
                    alpha: 0.7
                    entryEffect: ImageParticle.Fade
                }

                Gravity {
                    system: fireworksSystem
                    magnitude: 20
                    angle: 90
                }

                Turbulence {
                    system: fireworksSystem
                    strength: 10
                }
            }
        }
    }
}
