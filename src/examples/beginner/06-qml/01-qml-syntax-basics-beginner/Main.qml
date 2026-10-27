// Main.qml — QML 语法基础综合演示
// 展示：对象声明、基础类型、id 引用、JavaScript 表达式

import QtQuick

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: "QML Syntax Basics"

    // --- 自定义属性（基础类型演示） ---
    property int clickCount: 0
    property real fillRatio: 0.0
    property string statusMessage: "Ready"
    property bool isHighlighted: false
    property color normalColor: "#f5f5f5"
    property color highlightColor: "#E3F2FD"

    // --- 背景矩形 ---
    Rectangle {
        id: background
        anchors.fill: parent
        color: isHighlighted ? highlightColor : normalColor

        // 左侧信息面板
        Rectangle {
            id: infoPanel
            width: parent.width * 0.4
            height: parent.height - 40
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffffff"
            radius: 8

            border.color: "#e0e0e0"
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                Text {
                    text: "QML Basic Types"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#333333"
                }

                Text {
                    text: "int (clickCount): " + clickCount
                    font.pixelSize: 14
                    color: "#666666"
                }

                Text {
                    text: "real (fillRatio): " + fillRatio.toFixed(2)
                    font.pixelSize: 14
                    color: "#666666"
                }

                Text {
                    text: "string: " + statusMessage
                    font.pixelSize: 14
                    color: "#666666"
                }

                Text {
                    text: "bool (highlighted): " + isHighlighted
                    font.pixelSize: 14
                    color: "#666666"
                }

                Text {
                    text: "color: " + (isHighlighted ? highlightColor.toString() : normalColor.toString())
                    font.pixelSize: 14
                    color: "#666666"
                }
            }
        }

        // 右侧进度可视化
        Rectangle {
            id: progressPanel
            width: parent.width * 0.5
            height: parent.height - 40
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffffff"
            radius: 8
            border.color: "#e0e0e0"
            border.width: 1

            // 进度条（fillRatio 的可视化）
            Rectangle {
                id: progressBar
                anchors.fill: parent
                anchors.margins: 20
                color: "transparent"

                Rectangle {
                    id: progressFill
                    width: parent.width * fillRatio
                    height: 40
                    anchors.bottom: parent.bottom
                    color: {
                        // JavaScript 表达式：根据进度选择颜色
                        if (fillRatio < 0.33) return "#F44336";
                        if (fillRatio < 0.66) return "#FF9800";
                        return "#4CAF50";
                    }
                    radius: 4
                }

                Text {
                    anchors.centerIn: parent
                    text: {
                        if (fillRatio >= 1.0) return "Full!";
                        return "Fill: " + Math.round(fillRatio * 100) + "%";
                    }
                    font.pixelSize: 24
                    font.bold: true
                    color: "#333333"
                }
            }
        }
    }

    // --- 全局鼠标交互 ---
    MouseArea {
        anchors.fill: parent
        onClicked: function(mouse) {
            clickCount = clickCount + 1;
            isHighlighted = !isHighlighted;

            // 计算填充比例：点击位置 x 占窗口宽度的百分比
            fillRatio = Math.min(mouse.x / root.width, 1.0);

            statusMessage = "Click #" + clickCount
                + " at (" + Math.round(mouse.x) + ", " + Math.round(mouse.y) + ")";

            console.log("Click event:", JSON.stringify({
                x: mouse.x,
                y: mouse.y,
                count: clickCount,
                ratio: fillRatio.toFixed(2)
            }));
        }
    }
}
