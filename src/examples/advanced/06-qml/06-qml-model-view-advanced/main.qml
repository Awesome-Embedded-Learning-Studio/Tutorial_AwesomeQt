// main.qml — QML UI for the Model/View Advanced example.
//
// Demonstrates: ListView section headers, SortFilterProxyModel driven from
// a TextField, and ContactModel as the source data.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    title: qsTr("QML Model/View Advanced")
    width: 800
    height: 600
    visible: true

    // --- Source model and filter proxy, instantiated in QML ---

    ContactModel {
        id: contactModel
    }

    SortFilterModel {
        id: proxyModel
        sourceModel: contactModel
        filterString: searchField.text
    }

    // --- Layout ---

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Search bar
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Search:")
            }

            TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: qsTr("Type a name to filter…")
            }
        }

        // Contact list with section headers grouped by category
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            model: proxyModel

            // Group contacts by the "category" role
            section.property: "category"
            section.criteria: ViewSection.FullString
            section.delegate: Rectangle {
                required property string section
                width: listView.width
                height: 36
                color: "#e0e0e0"

                Label {
                    text: parent.section
                    font.bold: true
                    font.pixelSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                }
            }

            // Individual contact row
            delegate: ItemDelegate {
                required property string name
                required property string phone
                width: listView.width

                text: name + "  —  " + phone

                onClicked: {
                    // Show a brief notification when a contact is tapped
                    statusLabel.text = qsTr("Selected: %1").arg(name)
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        // Status bar showing filtered count
        Label {
            id: statusLabel
            Layout.fillWidth: true
            text: qsTr("Showing %1 of %2 contacts")
                    .arg(proxyModel.rowCount())
                    .arg(contactModel.rowCount())
        }
    }
}
