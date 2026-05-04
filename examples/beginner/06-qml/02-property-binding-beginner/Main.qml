// Main.qml — 属性绑定与响应式数据流综合演示
// 展示：属性绑定、自定义属性、onPropertyChanged、绑定断裂与修复

import QtQuick

Window {
    id: root
    width: 700
    height: 500
    visible: true
    title: "Property Binding Demo"

    // --- 自定义属性 ---
    property int sliderValue: 50
    property string statusText: "Normal"
    property color themeColor: "#2196F3"
    property bool useDarkMode: false
    property real scaleFactor: 1.0

    // --- 信号处理器 ---
    onSliderValueChanged: function() {
        if (sliderValue < 30) {
            statusText = "Low";
        } else if (sliderValue < 70) {
            statusText = "Normal";
        } else {
            statusText = "High";
        }
        console.log("Slider value changed:", sliderValue, "Status:", statusText);
    }

    onUseDarkModeChanged: function() {
        themeColor = useDarkMode ? "#BB86FC" : "#2196F3";
        console.log("Theme changed, dark mode:", useDarkMode);
    }

    // --- 背景层 ---
    Rectangle {
        anchors.fill: parent
        color: useDarkMode ? "#1a1a2e" : "#f5f5f5"
    }

    // --- 顶部标题栏 ---
    Rectangle {
        id: titleBar
        width: parent.width
        height: 60
        color: themeColor

        Text {
            anchors.centerIn: parent
            text: "Property Binding Demo"
            font.pixelSize: 22
            font.bold: true
            color: "#ffffff"
        }
    }

    // --- 主内容区 ---
    Item {
        anchors.top: titleBar.bottom
        anchors.bottom: controlBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20

        // 左侧：值可视化
        Rectangle {
            id: gaugePanel
            width: parent.width * 0.45
            height: parent.height
            anchors.left: parent.left
            color: useDarkMode ? "#16213e" : "#ffffff"
            radius: 8
            border.color: useDarkMode ? "#333366" : "#e0e0e0"
            border.width: 1

            Rectangle {
                id: gaugeFill
                width: parent.width * 0.8
                height: 30
                anchors.centerIn: parent
                radius: 15
                color: useDarkMode ? "#0f3460" : "#e8eaf6"
                clip: true

                Rectangle {
                    width: parent.width * (sliderValue / 100)
                    height: parent.height
                    radius: 15
                    color: {
                        if (sliderValue < 30) return "#F44336";
                        if (sliderValue < 70) return themeColor;
                        return "#4CAF50";
                    }
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: gaugeFill.bottom
                anchors.topMargin: 16
                text: sliderValue + "%"
                font.pixelSize: 36
                font.bold: true
                color: useDarkMode ? "#e0e0e0" : "#333333"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: gaugeFill.bottom
                anchors.topMargin: 70
                text: "Status: " + statusText
                font.pixelSize: 16
                color: useDarkMode ? "#aaaaaa" : "#666666"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16
                text: "scaleFactor: " + scaleFactor.toFixed(2)
                font.pixelSize: 13
                color: useDarkMode ? "#888888" : "#999999"
            }
        }

        // 右侧：属性信息面板
        Rectangle {
            id: infoPanel
            width: parent.width * 0.45
            height: parent.height
            anchors.right: parent.right
            color: useDarkMode ? "#16213e" : "#ffffff"
            radius: 8
            border.color: useDarkMode ? "#333366" : "#e0e0e0"
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Text {
                    text: "Property Values"
                    font.pixelSize: 18
                    font.bold: true
                    color: useDarkMode ? "#e0e0e0" : "#333333"
                }

                Text {
                    text: "sliderValue (int): " + sliderValue
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "statusText (string): " + statusText
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "themeColor (color): " + themeColor.toString()
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "useDarkMode (bool): " + useDarkMode
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "scaleFactor (real): " + scaleFactor.toFixed(2)
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: useDarkMode ? "#333366" : "#e0e0e0"
                }

                Text {
                    text: "Window: " + root.width + " x " + root.height
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "Gauge width: " + Math.round(gaugePanel.width * 0.8)
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }
            }
        }
    }

    // --- 底部控制栏 ---
    Rectangle {
        id: controlBar
        width: parent.width
        height: 120
        anchors.bottom: parent.bottom
        color: useDarkMode ? "#16213e" : "#ffffff"
        border.color: useDarkMode ? "#333366" : "#e0e0e0"
        border.width: 1

        Row {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 20

            Column {
                width: parent.width * 0.4
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Slider Value"
                    font.pixelSize: 14
                    font.bold: true
                    color: useDarkMode ? "#e0e0e0" : "#333333"
                }

                Rectangle {
                    width: parent.width
                    height: 20
                    radius: 10
                    color: useDarkMode ? "#0f3460" : "#e0e0e0"

                    Rectangle {
                        width: parent.width * (sliderValue / 100)
                        height: parent.height
                        radius: 10
                        color: themeColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        onMouseXChanged: function(mouse) {
                            if (pressed) {
                                var ratio = Math.max(0, Math.min(1, mouse.x / width));
                                sliderValue = Math.round(ratio * 100);
                            }
                        }
                    }
                }
            }

            Column {
                width: parent.width * 0.25
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Scale Factor: " + scaleFactor.toFixed(1)
                    font.pixelSize: 14
                    font.bold: true
                    color: useDarkMode ? "#e0e0e0" : "#333333"
                }

                Row {
                    spacing: 8

                    Rectangle {
                        width: 60
                        height: 36
                        radius: 6
                        color: themeColor

                        Text {
                            anchors.centerIn: parent
                            text: "Smaller"
                            font.pixelSize: 12
                            color: "#ffffff"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: function() {
                                scaleFactor = Math.max(0.5, scaleFactor - 0.1);
                            }
                        }
                    }

                    Rectangle {
                        width: 60
                        height: 36
                        radius: 6
                        color: themeColor

                        Text {
                            anchors.centerIn: parent
                            text: "Larger"
                            font.pixelSize: 12
                            color: "#ffffff"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: function() {
                                scaleFactor = Math.min(2.0, scaleFactor + 0.1);
                            }
                        }
                    }
                }
            }

            Column {
                width: parent.width * 0.25
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Theme"
                    font.pixelSize: 14
                    font.bold: true
                    color: useDarkMode ? "#e0e0e0" : "#333333"
                }

                Rectangle {
                    width: 120
                    height: 36
                    radius: 6
                    color: useDarkMode ? "#BB86FC" : "#e8eaf6"
                    border.color: themeColor
                    border.width: 2

                    Text {
                        anchors.centerIn: parent
                        text: useDarkMode ? "Dark Mode" : "Light Mode"
                        font.pixelSize: 13
                        color: useDarkMode ? "#ffffff" : themeColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: function() {
                            useDarkMode = !useDarkMode;
                        }
                    }
                }
            }
        }
    }
}
