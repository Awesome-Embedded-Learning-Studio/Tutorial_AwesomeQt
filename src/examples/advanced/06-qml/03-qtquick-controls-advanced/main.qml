// Qt Quick Controls 进阶示例
// 演示：Material 主题切换、自定义按钮模板、Palette 取色

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    visible: true
    width: 800
    height: 600
    title: "Qt Quick Controls Advanced"

    // Material 主题与强调色配置
    Material.theme: darkModeSwitch.checked ? Material.Dark : Material.Light
    Material.accent: accentCombo.currentValue
    Material.primary: Material.BlueGrey

    // ================================================================
    // SystemPalette：读取当前主题的系统调色板颜色
    // ================================================================
    SystemPalette {
        id: sysPalette
        colorGroup: SystemPalette.Active
    }

    // ================================================================
    // 主内容区域
    // ================================================================
    Flickable {
        anchors.fill: parent
        contentHeight: mainColumn.implicitHeight + 40
        clip: true

        ColumnLayout {
            id: mainColumn
            anchors.fill: parent
            anchors.margins: 24
            spacing: 20

            // ---------- 标题 ----------
            Label {
                text: "Qt Quick Controls Advanced Demo"
                font.pixelSize: 26
                font.bold: true
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            Label {
                text: "Material theme switching, custom control template, and Palette"
                font.pixelSize: 14
                opacity: 0.7
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // ---------- 主题切换 ----------
           GroupBox {
                Layout.fillWidth: true
                title: "Theme Control"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    // Dark / Light 主题切换开关
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: "Dark Theme:"
                            Layout.preferredWidth: 140
                        }

                        Switch {
                            id: darkModeSwitch
                            checked: true
                            text: checked ? "Dark" : "Light"
                        }
                    }

                    // 强调色选择
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: "Accent Color:"
                            Layout.preferredWidth: 140
                        }

                        ComboBox {
                            id: accentCombo
                            model: [
                                { value: Material.Red,    text: "Red" },
                                { value: Material.Blue,   text: "Blue" },
                                { value: Material.Green,  text: "Green" },
                                { value: Material.Orange, text: "Orange" },
                                { value: Material.Purple, text: "Purple" },
                                { value: Material.Cyan,   text: "Cyan" },
                                { value: Material.Pink,   text: "Pink" }
                            ]
                            textRole: "text"
                            currentIndex: 1

                            // 每个委托项显示色块 + 文字
                            delegate: ItemDelegate {
                                width: accentCombo.width
                                contentItem: RowLayout {
                                    spacing: 8
                                    Rectangle {
                                        width: 16
                                        height: 16
                                        radius: 3
                                        color: modelData.value
                                    }
                                    Label {
                                        text: modelData.text
                                    }
                                }
                                highlighted: accentCombo.highlightedIndex === index
                            }
                        }
                    }
                }
            }

            // ---------- 基础控件演示 ----------
            GroupBox {
                Layout.fillWidth: true
                title: "Styled Controls"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    // Slider
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: "Slider:"
                            Layout.preferredWidth: 140
                        }

                        Slider {
                            id: demoSlider
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 42
                        }

                        Label {
                            text: Math.round(demoSlider.value)
                            Layout.preferredWidth: 40
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    // TextField
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: "TextField:"
                            Layout.preferredWidth: 140
                        }

                        TextField {
                            id: demoTextField
                            Layout.fillWidth: true
                            placeholderText: "Type something here..."
                            placeholderTextColor: Material.color(
                                Material.Grey,
                                Material.Shade400
                            )
                            color: Material.foreground

                            // 右侧清除按钮
                            rightPadding: clearBtn.implicitWidth + 8

                            ToolButton {
                                id: clearBtn
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                text: "✕"
                                font.pixelSize: 14
                                opacity: demoTextField.length > 0 ? 1.0 : 0.0
                                onClicked: demoTextField.clear()

                                Behavior on opacity {
                                    NumberAnimation { duration: 150 }
                                }
                            }
                        }
                    }

                    // ComboBox
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: "ComboBox:"
                            Layout.preferredWidth: 140
                        }

                        ComboBox {
                            id: demoComboBox
                            Layout.fillWidth: true
                            model: ["Option A", "Option B", "Option C", "Option D"]
                            editable: false
                        }
                    }

                    // ProgressBar
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: "Progress:"
                            Layout.preferredWidth: 140
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: demoSlider.value
                        }

                        Label {
                            text: Math.round(demoSlider.value) + "%"
                            Layout.preferredWidth: 45
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }

            // ---------- 自定义按钮模板 ----------
            GroupBox {
                Layout.fillWidth: true
                title: "Custom Button Template"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Label {
                        text: "Demonstrates background + contentItem customization:"
                        font.italic: true
                        opacity: 0.7
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        // 自定义圆角渐变按钮
                        Button {
                            text: "Rounded Gradient"
                            Layout.preferredHeight: 44

                            // background 控制按钮外观
                            background: Rectangle {
                                radius: height / 2
                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop {
                                        position: 0.0
                                        color: Material.accent
                                    }
                                    GradientStop {
                                        position: 1.0
                                        color: Qt.lighter(Material.accent, 1.3)
                                    }
                                }
                                opacity: parent.pressed ? 0.7 : 1.0

                                // 点击时添加缩放动画
                                Behavior on opacity {
                                    NumberAnimation { duration: 100 }
                                }
                            }

                            // contentItem 控制文字渲染
                            contentItem: Label {
                                text: parent.text
                                font.pixelSize: 14
                                font.bold: true
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        // 自定义描边按钮
                        Button {
                            text: "Outlined"
                            Layout.preferredHeight: 44

                            background: Rectangle {
                                radius: 6
                                border.color: Material.accent
                                border.width: 2
                                color: parent.hovered
                                    ? Qt.rgba(Material.accent.r,
                                              Material.accent.g,
                                              Material.accent.b, 0.1)
                                    : "transparent"
                            }

                            contentItem: Label {
                                text: parent.text
                                font.pixelSize: 14
                                color: Material.accent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        // 自定义图标按钮
                        Button {
                            Layout.preferredWidth: 44
                            Layout.preferredHeight: 44

                            background: Rectangle {
                                radius: 22
                                color: parent.pressed
                                    ? Qt.rgba(Material.accent.r,
                                              Material.accent.g,
                                              Material.accent.b, 0.3)
                                    : Qt.rgba(Material.accent.r,
                                              Material.accent.g,
                                              Material.accent.b, 0.12)
                            }

                            contentItem: Label {
                                text: "★"                   // 星号 Unicode
                                font.pixelSize: 22
                                color: Material.accent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }

            // ---------- SystemPalette 展示 ----------
            GroupBox {
                Layout.fillWidth: true
                title: "SystemPalette Colors"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    Label {
                        text: "Colors read from SystemPalette (responds to theme changes):"
                        font.italic: true
                        opacity: 0.7
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    // 使用 Flow 自动换行展示色块
                    Flow {
                        Layout.fillWidth: true
                        spacing: 10

                        // 辅助函数：用 Repeater 生成色块
                        Repeater {
                            model: [
                                { name: "window",        color: sysPalette.window },
                                { name: "windowText",    color: sysPalette.windowText },
                                { name: "base",          color: sysPalette.base },
                                { name: "text",          color: sysPalette.text },
                                { name: "button",        color: sysPalette.button },
                                { name: "buttonText",    color: sysPalette.buttonText },
                                { name: "highlight",     color: sysPalette.highlight },
                                { name: "highlightedText", color: sysPalette.highlightedText },
                                { name: "mid",           color: sysPalette.mid },
                                { name: "light",         color: sysPalette.light },
                                { name: "midlight",      color: sysPalette.midlight },
                                { name: "dark",          color: sysPalette.dark },
                                { name: "shadow",        color: sysPalette.shadow }
                            ]

                            delegate: Rectangle {
                                width: 90
                                height: 60
                                radius: 6
                                color: modelData.color
                                border.color: Material.foreground
                                border.width: 1

                                Column {
                                    anchors.centerIn: parent
                                    spacing: 2

                                    Rectangle {
                                        width: 30
                                        height: 16
                                        radius: 3
                                        color: modelData.color
                                        border.color: "grey"
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }

                                    Label {
                                        text: modelData.name
                                        font.pixelSize: 9
                                        color: sysPalette.windowText
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // 底部留白
            Item {
                Layout.fillWidth: true
                height: 16
            }
        }

        // 滚动条
        ScrollBar.vertical: ScrollBar {}
    }

    // ---------- 状态栏 ----------
    footer: ToolBar {
        position: ToolBar.Footer
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12

            Label {
                text: "Style: " + (typeof Material !== "undefined" ? "Material" : "Default")
                font.pixelSize: 12
                opacity: 0.7
            }

            Item { Layout.fillWidth: true }

            Label {
                text: darkModeSwitch.checked ? "Theme: Dark" : "Theme: Light"
                font.pixelSize: 12
                opacity: 0.7
            }
        }
    }
}
