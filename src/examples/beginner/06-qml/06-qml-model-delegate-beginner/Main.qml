// Main.qml — QML Model/Delegate 数据驱动视图综合演示

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 700
    height: 600
    visible: true
    title: "QML Model/Delegate Demo"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // 标题
        Label {
            text: "Fruit Store (C++ Model)"
            font.pixelSize: 22
            font.bold: true
        }

        // 操作面板
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: nameInput
                Layout.preferredWidth: 100
                placeholderText: "Name"
            }
            ComboBox {
                id: colorInput
                Layout.preferredWidth: 100
                model: ["#e74c3c", "#f1c40f", "#9b59b6", "#e67e22", "#3498db", "#2ecc71"]
                displayText: "Color"
            }
            SpinBox {
                id: priceInput
                from: 0
                to: 999
                value: 10
                editable: true
                textFromValue: function(value) { return "$" + (value / 1).toFixed(1) }
                valueFromText: function(text) { return parseFloat(text.replace("$", "")) * 1 }
            }
            CheckBox {
                id: stockCheck
                text: "In Stock"
                checked: true
            }
            Button {
                text: "Add"
                highlighted: true
                onClicked: {
                    if (nameInput.text.length > 0) {
                        fruitModel.addFruit(nameInput.text, colorInput.currentText,
                                           priceInput.value, stockCheck.checked)
                        nameInput.text = ""
                    }
                }
            }
            Button {
                text: "Clear All"
                onClicked: fruitModel.clearAll()
            }
        }

        // 视图切换标签
        TabBar {
            id: viewTab
            Layout.fillWidth: true

            TabButton { text: "List View" }
            TabButton { text: "Grid View" }
        }

        // 列表视图
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: fruitModel
            spacing: 6
            clip: true
            visible: viewTab.currentIndex === 0

            delegate: Rectangle {
                width: listView.width
                height: 56
                color: "#ffffff"
                radius: 8
                border.color: "#e8e8e8"

                // 悬停效果
                property bool hovered: listMouse.containsMouse
                scale: hovered ? 1.01 : 1.0

                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 12

                    Rectangle {
                        width: 32
                        height: 32
                        radius: 16
                        color: model.fruitColor
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1

                        Text {
                            text: model.name
                            font.pixelSize: 14
                            font.bold: true
                            color: "#2c3e50"
                        }
                        Text {
                            text: "$" + model.price.toFixed(1)
                            font.pixelSize: 12
                            color: "#7f8c8d"
                        }
                    }

                    Text {
                        text: model.inStock ? "In Stock" : "Out of Stock"
                        font.pixelSize: 11
                        font.bold: true
                        color: model.inStock ? "#27ae60" : "#e74c3c"
                    }

                    Button {
                        text: "Del"
                        flat: true
                        font.pixelSize: 11
                        onClicked: fruitModel.removeFruit(index)
                    }
                }

                MouseArea {
                    id: listMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: console.log("Clicked:", model.name, "index:", index)
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        // 网格视图
        GridView {
            id: gridView
            Layout.fillWidth: true
            Layout.fillHeight: true
            cellWidth: width / 3
            cellHeight: 130
            model: fruitModel
            clip: true
            visible: viewTab.currentIndex === 1

            delegate: Rectangle {
                width: gridView.cellWidth - 10
                height: gridView.cellHeight - 10
                color: "#ffffff"
                radius: 10
                border.color: "#e8e8e8"
                anchors.horizontalCenter: parent.horizontalCenter

                Column {
                    anchors.centerIn: parent
                    spacing: 6

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: model.fruitColor
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: model.name
                        font.pixelSize: 13
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: "$" + model.price.toFixed(1)
                        font.pixelSize: 11
                        color: "#7f8c8d"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: model.inStock ? "In Stock" : "Out"
                        font.pixelSize: 10
                        font.bold: true
                        color: model.inStock ? "#27ae60" : "#e74c3c"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: fruitModel.removeFruit(index)
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        // 底部统计
        Label {
            Layout.fillWidth: true
            font.pixelSize: 12
            color: "#999"
            text: "Total: " + fruitModel.rowCount() + " items"
        }
    }
}
