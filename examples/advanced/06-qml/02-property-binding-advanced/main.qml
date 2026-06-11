// Copyright (C) 2026 The Tutorial_AwesomeQt Authors
// SPDX-License-Identifier: MIT

/// @file    main.qml
/// @brief   Demonstrates Binding element, Qt.binding(), and property alias.
///
/// @details Three demo sections:
///          1. Binding element with `when` condition for conditional bindings.
///          2. Qt.binding() to restore bindings broken by imperative assignment.
///          3. property alias for exposing internal item properties.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    title: "Property Binding Advanced"
    width: 800
    height: 500
    visible: true

    // ================================================================
    // Demo 3 helper: a reusable component with property alias
    // ================================================================

    // @note property alias exposes an internal item's property to the outside,
    // making it bindable and assignable from the consumer side. Unlike a regular
    // property, an alias does NOT allocate extra storage — it is a direct
    // reference to the aliased target.
    component ColorBox : Rectangle {
        id: colorBox

        width: 120
        height: 40
        radius: 6

        // @note Alias the fill color to Rectangle's built-in `color` property.
        // We do NOT set `color` directly here — the alias IS the color setter.
        // Setting both `color: "..."` and `boxColor: "..."` would cause the
        // QML compiler to report "Property value set multiple times".
        property alias boxColor: colorBox.color

        // Alias the internal label's text so callers can set it declaratively
        property alias label: innerLabel.text

        Text {
            id: innerLabel
            anchors.centerIn: parent
            text: "Label"
            font.pixelSize: 14
        }
    }

    // ================================================================
    // Main layout
    // ================================================================

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 24

        // -------- Demo 1: Binding element with `when` condition --------

        Label {
            text: "Demo 1: Binding Element with `when` Condition"
            font.bold: true
            font.pixelSize: 18
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Rectangle {
                id: demo1Rect
                width: 120
                height: 80
                radius: 8

                // @note When neither Binding's `when` is true, the property
                // retains its last value — it does NOT reset to a default.
                color: "gray"

                Text {
                    anchors.centerIn: parent
                    text: "Watch my color"
                    font.pixelSize: 12
                }

                // @note The Binding element attaches a conditional binding to a
                // target property. When `when` evaluates to true, the binding
                // takes effect; when false, it is disabled and the property is
                // free to receive other values.
                Binding {
                    target: demo1Rect
                    property: "color"
                    value: "red"
                    when: toggleRed.checked
                }

                Binding {
                    target: demo1Rect
                    property: "color"
                    value: "blue"
                    when: !toggleRed.checked
                }
            }

            Switch {
                id: toggleRed
                checked: false
                text: "Red / Blue"
            }

            Label {
                text: toggleRed.checked ? "Binding active: red" : "Binding active: blue"
                font.pixelSize: 13
                Layout.alignment: Qt.AlignVCenter
            }
        }

        // -------- Demo 2: Qt.binding() for imperative binding restoration --------

        Label {
            text: "Demo 2: Qt.binding() — Restore Broken Binding"
            font.bold: true
            font.pixelSize: 18
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Rectangle {
                id: demo2Rect
                radius: 8
                color: "#4CAF50"

                // @note This is a declarative binding: the width tracks the slider
                // value automatically. Any imperative assignment will BREAK this
                // binding permanently unless explicitly restored.
                width: slider.value * 2
                height: 80

                Text {
                    anchors.centerIn: parent
                    text: "w: " + parent.width.toFixed(0)
                    font.pixelSize: 12
                    color: "white"
                }
            }

            Slider {
                id: slider
                from: 40
                to: 200
                value: 100
            }

            Button {
                text: "Break Binding"
                onClicked: {
                    // @note Imperative assignment (JS =) destroys the declarative
                    // binding on width. After this, the slider no longer controls
                    // the rectangle width.
                    demo2Rect.width = 300;
                    statusLabel.text = "Binding BROKEN — width fixed at 300";
                }
            }

            Button {
                text: "Restore Binding"
                onClicked: {
                    // @note Qt.binding() creates a new JS binding object and
                    // re-attaches it to the property. This is the ONLY way to
                    // restore a declarative binding from imperative code.
                    demo2Rect.width = Qt.binding(function() {
                        return slider.value * 2;
                    });
                    statusLabel.text = "Binding RESTORED — slider controls width";
                }
            }

            Label {
                id: statusLabel
                text: "Binding active — slider controls width"
                font.pixelSize: 13
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
        }

        // -------- Demo 3: property alias usage --------

        Label {
            text: "Demo 3: property alias — Expose Internal Properties"
            font.bold: true
            font.pixelSize: 18
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            // @note Here we set `label` and `boxColor` on ColorBox instances.
            // These are property aliases — they directly write to the aliased
            // internal properties without creating intermediate storage.
            ColorBox {
                label: "Alias Demo A"
                boxColor: "#E91E63"
            }

            ColorBox {
                label: "Alias Demo B"
                boxColor: "#3F51B5"
            }

            ColorBox {
                id: dynamicBox
                label: "Bound via Alias"
                // @note An alias can also participate in bindings, just like
                // a regular property. This color will update reactively.
                boxColor: aliasToggle.checked ? "#FF9800" : "#9C27B0"
            }

            Switch {
                id: aliasToggle
                checked: false
                text: "Toggle"
            }
        }

        Item {
            // Spacer to push content up
            Layout.fillHeight: true
        }
    }
}
