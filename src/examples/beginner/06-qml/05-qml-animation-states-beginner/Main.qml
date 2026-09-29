// Main.qml — QML 动画与状态机综合演示
// 展示：NumberAnimation / ColorAnimation / Behavior on / State + Transition
//        SequentialAnimation / ParallelAnimation

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 800
    height: 700
    visible: true
    title: "QML Animation & States Demo"

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: Math.max(window.width, 760)
            spacing: 32

            // ====== 标题 ======
            Label {
                text: "QML Animation & States Demo"
                font.pixelSize: 28
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 24
            }

            // ====== 1. NumberAnimation 位移动画 ======
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                title: "NumberAnimation - Position"

                ColumnLayout {
                    anchors.fill: parent

                    Label {
                        text: "Click the ball to toggle animation"
                        font.pixelSize: 12
                        color: "#888"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 100
                        color: "#f0f0f0"
                        radius: 8
                        clip: true

                        Rectangle {
                            id: ball
                            width: 60
                            height: 60
                            radius: 30
                            color: "#3498db"
                            x: 20
                            y: 20

                            // 阴影效果
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: -2
                                radius: parent.radius + 2
                                color: "transparent"
                                border.color: "#20000000"
                                z: -1
                            }

                            Text {
                                anchors.centerIn: parent
                                text: "Click"
                                color: "#ffffff"
                                font.pixelSize: 12
                                font.bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: moveAnim.running = !moveAnim.running
                            }

                            NumberAnimation on x {
                                id: moveAnim
                                from: 20
                                to: parent.width - 80
                                duration: 1200
                                easing.type: Easing.InOutCubic
                                running: false
                                loops: Animation.Infinite
                            }

                            // 垂直弹跳
                            SequentialAnimation on y {
                                loops: Animation.Infinite
                                running: moveAnim.running
                                NumberAnimation {
                                    from: 20
                                    to: 30
                                    duration: 300
                                    easing.type: Easing.OutCubic
                                }
                                NumberAnimation {
                                    from: 30
                                    to: 20
                                    duration: 300
                                    easing.type: Easing.InCubic
                                }
                            }
                        }
                    }
                }
            }

            // ====== 2. ColorAnimation 颜色渐变 ======
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                title: "ColorAnimation - Color Transition"

                ColumnLayout {
                    anchors.fill: parent

                    Label {
                        text: "Click to cycle colors"
                        font.pixelSize: 12
                        color: "#888"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Rectangle {
                            id: colorBox
                            Layout.fillWidth: true
                            height: 80
                            color: "#e74c3c"
                            radius: 8

                            Behavior on color {
                                ColorAnimation { duration: 500; easing.type: Easing.InOutQuad }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: colorBox.color
                                color: "#ffffff"
                                font.pixelSize: 14
                                font.bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                property int colorIndex: 0
                                onClicked: {
                                    var colors = ["#e74c3c", "#3498db", "#2ecc71", "#f39c12", "#9b59b6"]
                                    colorIndex = (colorIndex + 1) % colors.length
                                    colorBox.color = colors[colorIndex]
                                }
                            }
                        }
                    }
                }
            }

            // ====== 3. Behavior on 悬停效果卡片 ======
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                title: "Behavior on - Hover Effects"

                RowLayout {
                    anchors.fill: parent
                    spacing: 16

                    Repeater {
                        model: [
                            { title: "Card A", desc: "Hover me", color: "#3498db" },
                            { title: "Card B", desc: "Or me", color: "#e74c3c" },
                            { title: "Card C", desc: "Me too", color: "#2ecc71" }
                        ]

                        Rectangle {
                            Layout.fillWidth: true
                            height: 120
                            color: "#ffffff"
                            radius: 12
                            border.color: "#e0e0e0"
                            border.width: 1

                            scale: cardMouse.containsMouse ? 1.05 : 1.0
                            opacity: cardMouse.containsMouse ? 1.0 : 0.8

                            Behavior on scale {
                                NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
                            }
                            Behavior on opacity {
                                NumberAnimation { duration: 150 }
                            }

                            Column {
                                anchors.centerIn: parent
                                spacing: 4

                                Rectangle {
                                    width: 40
                                    height: 40
                                    radius: 20
                                    color: modelData.color
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }

                                Text {
                                    text: modelData.title
                                    font.pixelSize: 14
                                    font.bold: true
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                                Text {
                                    text: modelData.desc
                                    font.pixelSize: 12
                                    color: "#888"
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }

                            MouseArea {
                                id: cardMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }
            }

            // ====== 4. State + Transition 展开/折叠面板 ======
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                title: "State + Transition - Expandable Panel"

                ColumnLayout {
                    anchors.fill: parent

                    Rectangle {
                        id: panel
                        Layout.fillWidth: true
                        height: 50
                        color: "#2c3e50"
                        radius: 8
                        clip: true

                        state: "collapsed"

                        states: [
                            State {
                                name: "collapsed"
                                PropertyChanges { target: panel; height: 50 }
                                PropertyChanges { target: panelContent; opacity: 0 }
                                PropertyChanges { target: panelArrow; rotation: 0 }
                            },
                            State {
                                name: "expanded"
                                PropertyChanges { target: panel; height: 180 }
                                PropertyChanges { target: panelContent; opacity: 1.0 }
                                PropertyChanges { target: panelArrow; rotation: 180 }
                            }
                        ]

                        transitions: [
                            Transition {
                                from: "collapsed"
                                to: "expanded"
                                ParallelAnimation {
                                    NumberAnimation {
                                        target: panel
                                        property: "height"
                                        duration: 300
                                        easing.type: Easing.OutCubic
                                    }
                                    NumberAnimation {
                                        target: panelContent
                                        property: "opacity"
                                        duration: 300
                                    }
                                    NumberAnimation {
                                        target: panelArrow
                                        property: "rotation"
                                        duration: 300
                                    }
                                }
                            },
                            Transition {
                                from: "expanded"
                                to: "collapsed"
                                ParallelAnimation {
                                    NumberAnimation {
                                        target: panel
                                        property: "height"
                                        duration: 250
                                        easing.type: Easing.InCubic
                                    }
                                    NumberAnimation {
                                        target: panelContent
                                        property: "opacity"
                                        duration: 150
                                    }
                                    NumberAnimation {
                                        target: panelArrow
                                        property: "rotation"
                                        duration: 250
                                    }
                                }
                            }
                        ]

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 8

                            Text {
                                text: "Click to expand / collapse"
                                color: "#ecf0f1"
                                font.pixelSize: 14
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            Text {
                                id: panelArrow
                                text: "\u25BC"
                                color: "#ecf0f1"
                                font.pixelSize: 14
                            }
                        }

                        Text {
                            id: panelContent
                            anchors.fill: parent
                            anchors.margins: 16
                            anchors.topMargin: 50
                            text: "This is the expanded content area.\n" +
                                  "The panel smoothly transitions between collapsed and expanded states.\n" +
                                  "Height, opacity, and rotation are animated in parallel."
                            color: "#bdc3c7"
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                panel.state = (panel.state === "collapsed") ? "expanded" : "collapsed"
                            }
                        }
                    }
                }
            }

            // ====== 5. SequentialAnimation 脉冲效果 ======
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                title: "SequentialAnimation - Pulse Effect"

                ColumnLayout {
                    anchors.fill: parent

                    Label {
                        text: "Click the button to see pulse animation"
                        font.pixelSize: 12
                        color: "#888"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 80
                        color: "#f8f9fa"
                        radius: 8

                        Rectangle {
                            id: pulseBtn
                            width: 140
                            height: 48
                            radius: 8
                            color: "#e74c3c"
                            anchors.centerIn: parent

                            Text {
                                anchors.centerIn: parent
                                text: "Pulse Me"
                                color: "#ffffff"
                                font.pixelSize: 14
                                font.bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: pulseAnim.start()
                            }

                            SequentialAnimation {
                                id: pulseAnim

                                NumberAnimation {
                                    target: pulseBtn
                                    property: "scale"
                                    to: 1.2
                                    duration: 120
                                    easing.type: Easing.OutCubic
                                }
                                NumberAnimation {
                                    target: pulseBtn
                                    property: "scale"
                                    to: 0.95
                                    duration: 80
                                    easing.type: Easing.InOutCubic
                                }
                                NumberAnimation {
                                    target: pulseBtn
                                    property: "scale"
                                    to: 1.05
                                    duration: 60
                                    easing.type: Easing.OutCubic
                                }
                                NumberAnimation {
                                    target: pulseBtn
                                    property: "scale"
                                    to: 1.0
                                    duration: 40
                                    easing.type: Easing.InCubic
                                }
                            }
                        }
                    }
                }
            }

            // 底部留白
            Item {
                Layout.fillHeight: true
                Layout.minimumHeight: 24
            }
        }
    }
}
