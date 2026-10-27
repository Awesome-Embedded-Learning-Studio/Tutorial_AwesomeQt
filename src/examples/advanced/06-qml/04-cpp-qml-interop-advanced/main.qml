// QML main UI — 消费 C++ TaskModel 与 AppController
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CppQmlInteropAdvanced

ApplicationWindow {
    id: root
    visible: true
    width: 420
    height: 600
    title: qsTr("C++ / QML Interop — Advanced")

    // ─── 顶栏：时间 + 问候 ───────────────────────────────────
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8

            Label {
                text: AppController.formatTime()
                font.bold: true
                Layout.alignment: Qt.AlignLeft
            }

            Label {
                text: AppController.greeting("Developer")
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    // ─── 主内容 ───────────────────────────────────────────────
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // 已完成计数
        Label {
            text: qsTr("Completed: %1 / %2")
                    .arg(taskModel.completedCount())
                    .arg(taskModel.rowCount())
            font.italic: true
        }

        // 任务列表，model 绑定到 C++ TaskModel 实例
        ListView {
            id: taskList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: taskModel
            spacing: 4

            delegate: RowLayout {
                width: taskList.width
                spacing: 8

                CheckBox {
                    checked: model.taskDone
                    onToggled: taskModel.toggleTask(model.index)
                }

                Label {
                    text: model.taskName
                    Layout.fillWidth: true
                    // 已完成的任务显示删除线
                    font.strikeout: model.taskDone
                }

                Label {
                    text: qsTr("P%1").arg(model.taskPriority)
                    color: model.taskPriority >= 4 ? "red" : "gray"
                }

                Button {
                    text: qsTr("Delete")
                    flat: true
                    onClicked: taskModel.removeTask(model.index)
                }
            }
        }

        // ─── 底部操作区 ────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: nameInput
                Layout.fillWidth: true
                placeholderText: qsTr("New task name...")
            }

            SpinBox {
                id: prioritySpin
                from: 1
                to: 5
                value: 2
            }

            Button {
                text: qsTr("Add")
                highlighted: true
                onClicked: {
                    if (nameInput.text.length > 0) {
                        taskModel.addTask(nameInput.text, prioritySpin.value)
                        nameInput.clear()
                    }
                }
            }
        }
    }
}
