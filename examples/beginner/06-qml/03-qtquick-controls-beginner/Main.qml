// Main.qml — Qt Quick Controls 组件综合演示
// 展示：ApplicationWindow、常用控件、布局、弹出组件

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "Qt Quick Controls Demo"

    // --- 状态属性 ---
    property string userName: ""
    property string selectedTheme: "System"
    property real volumeLevel: 0.5
    property bool notificationsEnabled: true
    property bool autoSaveEnabled: true
    property string selectedLanguage: "English"

    // --- 菜单栏 ---
    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem {
                text: "&Reset Settings"
                onTriggered: resetSettings()
            }
            MenuSeparator {}
            MenuItem {
                text: "&Quit"
                onTriggered: Qt.quit()
            }
        }

        Menu {
            title: "&Help"
            MenuItem {
                text: "&About"
                onTriggered: aboutDialog.open()
            }
        }
    }

    // --- 顶部工具栏 ---
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8

            Label {
                text: "Qt Quick Controls Demo"
                font.bold: true
                font.pixelSize: 16
                Layout.leftMargin: 8
            }

            Item { Layout.fillWidth: true }

            Label {
                text: "Theme:"
            }

            ComboBox {
                model: ["System", "Light", "Dark"]
                currentIndex: 0
                onActivated: function(index) {
                    selectedTheme = model[index]
                    console.log("Theme changed to:", selectedTheme)
                }
            }

            Button {
                text: "About"
                flat: true
                onClicked: aboutDialog.open()
            }
        }
    }

    // --- 中央内容区：GridLayout 表单 ---
    GridLayout {
        anchors.fill: parent
        anchors.margins: 24
        columns: 2
        rowSpacing: 16
        columnSpacing: 20

        Label {
            Layout.columnSpan: 2
            text: "User Settings"
            font.pixelSize: 24
            font.bold: true
        }

        Label {
            text: "Username:"
            font.pixelSize: 14
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        }
        TextField {
            id: nameInput
            Layout.fillWidth: true
            placeholderText: "Enter your username"
            text: userName
            onTextChanged: userName = text
        }

        Label {
            text: "Language:"
            font.pixelSize: 14
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        }
        ComboBox {
            id: languageCombo
            Layout.fillWidth: true
            model: ["English", "Chinese", "Japanese", "German"]
            currentIndex: 0
            onActivated: function(index) {
                selectedLanguage = model[index]
                console.log("Language:", selectedLanguage)
            }
        }

        Label {
            text: "Volume:"
            font.pixelSize: 14
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Slider {
                id: volumeSlider
                Layout.fillWidth: true
                from: 0
                to: 1.0
                value: volumeLevel
                stepSize: 0.05
                onValueChanged: volumeLevel = value
            }
            Label {
                text: Math.round(volumeLevel * 100) + "%"
                font.pixelSize: 14
                Layout.preferredWidth: 40
            }
        }

        Label {
            text: "Options:"
            font.pixelSize: 14
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            CheckBox {
                id: notificationCheck
                text: "Enable notifications"
                checked: notificationsEnabled
                onCheckedChanged: notificationsEnabled = checked
            }

            CheckBox {
                id: autoSaveCheck
                text: "Enable auto-save"
                checked: autoSaveEnabled
                onCheckedChanged: autoSaveEnabled = checked
            }

            CheckBox {
                text: "Show advanced options"
                checkable: true
                onCheckedChanged: console.log("Advanced:", checked)
            }
        }

        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            height: 1
            color: "#e0e0e0"
        }

        Label {
            text: "Preview:"
            font.pixelSize: 14
            font.bold: true
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: previewContent.height + 24
            color: "#f8f9fa"
            radius: 8
            border.color: "#dee2e6"
            border.width: 1

            Column {
                id: previewContent
                anchors.fill: parent
                anchors.margins: 12
                spacing: 4

                Text {
                    font.pixelSize: 13
                    text: "Name: " + (userName || "(not set)")
                    color: "#495057"
                }
                Text {
                    font.pixelSize: 13
                    text: "Language: " + selectedLanguage
                    color: "#495057"
                }
                Text {
                    font.pixelSize: 13
                    text: "Volume: " + Math.round(volumeLevel * 100) + "%"
                    color: "#495057"
                }
                Text {
                    font.pixelSize: 13
                    text: "Notifications: " + (notificationsEnabled ? "On" : "Off")
                    color: "#495057"
                }
                Text {
                    font.pixelSize: 13
                    text: "Auto-save: " + (autoSaveEnabled ? "On" : "Off")
                    color: "#495057"
                }
            }
        }

        RowLayout {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.topMargin: 8

            Item { Layout.fillWidth: true }

            Button {
                text: "Reset"
                onClicked: resetDialog.open()
            }

            Button {
                text: "Apply"
                highlighted: true
                onClicked: {
                    console.log("Settings applied:", JSON.stringify({
                        userName: userName,
                        language: selectedLanguage,
                        volume: volumeLevel,
                        notifications: notificationsEnabled,
                        autoSave: autoSaveEnabled
                    }))
                    applyPopup.open()
                }
            }
        }
    }

    // --- 底部状态栏 ---
    footer: ToolBar {
        RowLayout {
            anchors.fill: parent

            Label {
                text: "Ready"
                Layout.leftMargin: 10
            }

            Item { Layout.fillWidth: true }

            Label {
                text: userName ? "User: " + userName : "No user set"
                Layout.rightMargin: 10
                color: userName ? "#4CAF50" : "#999999"
            }
        }
    }

    // --- 关于对话框 ---
    Dialog {
        id: aboutDialog
        title: "About"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok

        Column {
            spacing: 8
            width: 280

            Text {
                font.pixelSize: 18
                font.bold: true
                text: "Qt Quick Controls Demo"
            }
            Text {
                font.pixelSize: 13
                color: "#666666"
                text: "Version 1.0\nBuilt with Qt 6.9.1\nA comprehensive demo of Qt Quick Controls components."
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }
    }

    // --- 重置确认对话框 ---
    Dialog {
        id: resetDialog
        title: "Reset Settings"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No

        Label {
            text: "Are you sure you want to reset all settings to default values?"
        }

        onAccepted: resetSettings()
    }

    // --- 应用成功弹出 ---
    Popup {
        id: applyPopup
        x: Math.round((parent.width - width) / 2)
        y: parent.height - height - 60
        width: 200
        height: 44
        padding: 8

        background: Rectangle {
            color: "#4CAF50"
            radius: 8
        }

        Text {
            anchors.centerIn: parent
            text: "Settings Applied"
            color: "#ffffff"
            font.pixelSize: 14
            font.bold: true
        }

        onOpened: closeTimer.start()

        Timer {
            id: closeTimer
            interval: 2000
            onTriggered: applyPopup.close()
        }
    }

    // --- 右键菜单 ---
    Menu {
        id: contextMenu

        MenuItem {
            text: "Reset to Default"
            onTriggered: resetSettings()
        }
        MenuSeparator {}
        MenuItem {
            text: "Show Current Settings"
            onTriggered: console.log("Current settings:", JSON.stringify({
                userName: userName,
                language: selectedLanguage,
                volume: volumeLevel,
                notifications: notificationsEnabled,
                autoSave: autoSaveEnabled,
                theme: selectedTheme
            }))
        }
    }

    // --- 右键触发 ---
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true
        onClicked: function(mouse) {
            contextMenu.popup()
            mouse.accepted = false
        }
    }

    // --- 辅助函数 ---
    function resetSettings()
    {
        nameInput.text = ""
        languageCombo.currentIndex = 0
        volumeSlider.value = 0.5
        notificationCheck.checked = true
        autoSaveCheck.checked = true
        console.log("Settings reset to defaults")
    }
}
