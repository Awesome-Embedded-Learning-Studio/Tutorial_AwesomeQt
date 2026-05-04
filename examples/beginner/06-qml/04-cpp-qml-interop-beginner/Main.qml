// Main.qml — C++ 与 QML 互操作综合演示

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 640
    height: 520
    visible: true
    title: "C++ / QML Interop Demo"

    // --- 通知弹窗状态 ---
    property string notificationText: ""
    property bool showNotification: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // 标题
        Label {
            text: "C++ & QML Interop Demo"
            font.pixelSize: 24
            font.bold: true
        }

        // 用户名输入
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 16
            rowSpacing: 12

            Label {
                text: "User Name:"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
            TextField {
                id: nameInput
                Layout.fillWidth: true
                placeholderText: "Type your name..."
                // 双向绑定：QML 输入 -> C++ 属性
                onTextChanged: appController.userName = text
                Component.onCompleted: text = appController.userName
            }

            Label {
                text: "Counter:"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: appController.counter
                    font.pixelSize: 28
                    font.bold: true
                    color: "#2c3e50"
                    Layout.preferredWidth: 60
                    horizontalAlignment: Text.AlignHCenter
                }

                Button {
                    text: "+1"
                    onClicked: appController.increment()
                }
                Button {
                    text: "Reset"
                    onClicked: appController.reset()
                }
            }

            Label {
                text: "Theme:"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
            ComboBox {
                id: themeCombo
                Layout.fillWidth: true
                model: ["#3498db", "#e74c3c", "#2ecc71", "#f39c12", "#9b59b6"]
                displayText: "Pick a color"
                onActivated: function(index) {
                    appController.themeColor = model[index]
                }
            }
        }

        // 分隔线
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#ddd"
        }

        // Greeting 区域：展示 C++ 方法的返回值
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: greetingLabel.height + 24
            color: appController.themeColor
            radius: 8

            Label {
                id: greetingLabel
                anchors.centerIn: parent
                text: appController.greeting()
                font.pixelSize: 16
                font.bold: true
                color: "#ffffff"
            }

            Behavior on color {
                ColorAnimation { duration: 300 }
            }
        }

        // 状态面板
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: statusContent.height + 24
            color: "#f8f9fa"
            radius: 8
            border.color: "#dee2e6"

            Column {
                id: statusContent
                anchors.fill: parent
                anchors.margins: 12
                spacing: 4

                Text {
                    font.pixelSize: 13
                    text: "userName: " + appController.userName
                    color: "#495057"
                }
                Text {
                    font.pixelSize: 13
                    text: "counter: " + appController.counter
                    color: "#495057"
                }
                Text {
                    font.pixelSize: 13
                    text: "themeColor: " + appController.themeColor
                    color: "#495057"
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    // 接收 C++ 端发射的 notificationRequested 信号
    Connections {
        target: appController
        function onNotificationRequested(message) {
            notificationText = message
            showNotification = true
            notificationHideTimer.start()
        }
    }

    // 通知弹出条
    Rectangle {
        id: notificationBar
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        width: notificationText.length * 9 + 40
        height: 40
        color: "#2c3e50"
        radius: 8
        opacity: showNotification ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 300 }
        }

        Text {
            anchors.centerIn: parent
            text: notificationText
            color: "#ffffff"
            font.pixelSize: 13
        }

        Timer {
            id: notificationHideTimer
            interval: 3000
            onTriggered: showNotification = false
        }
    }
}
