/// @file    main.qml
/// @brief   QML 异步编程主界面，三个 Tab 分别演示三种异步模式。
///
/// Tab 1: WorkerScript —— 后台线程素数计算，UI 保持响应
/// Tab 2: Loader —— 异步加载/卸载组件，显示加载指示器
/// Tab 3: XMLHttpRequest —— 网络请求演示 onreadystatechange 模式
/// 对应教程：进阶层 06-QML/07-异步工作线程与网络请求

import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: root
    visible: true
    width: 800
    height: 600
    title: qsTr("QML Async Advanced")

    /// @brief 当前选中的 Tab 索引
    header: TabBar {
        id: tabBar
        TabButton { text: qsTr("WorkerScript") }
        TabButton { text: qsTr("Loader Async") }
        TabButton { text: qsTr("XMLHttpRequest") }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        // ---- Tab 1: WorkerScript 后台素数计算 ----
        Item {
            /// WorkerScript 在独立线程中执行，不会阻塞 UI
            WorkerScript {
                id: worker
                source: "worker.js"

                onMessage: function(message) {
                    if (message.action === "result") {
                        resultLabel.text = qsTr("Found %1 primes below %2")
                            .arg(message.count)
                            .arg(message.limit);
                        computeButton.enabled = true;
                        computeButton.text = qsTr("Start Computation");
                    }
                }
            }

            Column {
                anchors.centerIn: parent
                spacing: 16
                width: parent.width * 0.6

                Label {
                    text: qsTr("WorkerScript Demo")
                    font.pixelSize: 20
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: qsTr("Compute primes in a background thread. " +
                               "The UI stays responsive during calculation.")
                    wrapMode: Text.WordWrap
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }

                Row {
                    spacing: 12
                    anchors.horizontalCenter: parent.horizontalCenter

                    Label {
                        text: qsTr("Upper limit:")
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    SpinBox {
                        id: limitSpinBox
                        from: 100
                        to: 10000000
                        value: 100000
                        stepSize: 10000
                        editable: true
                    }
                }

                Button {
                    id: computeButton
                    text: qsTr("Start Computation")
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: {
                        enabled = false;
                        text = qsTr("Computing...");
                        resultLabel.text = qsTr("Working... try resizing the window!");
                        // 发送消息到 WorkerScript 线程
                        worker.sendMessage({
                            action: "compute",
                            limit: limitSpinBox.value
                        });
                    }
                }

                Label {
                    id: resultLabel
                    text: qsTr("Press the button to start")
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                    wrapMode: Text.WordWrap
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // ---- Tab 2: Loader 异步加载 ----
        Item {
            Loader {
                id: asyncLoader
                anchors.fill: parent
                anchors.topMargin: 160
                asynchronous: true  // 在独立线程中加载组件

                // 异步加载时显示 BusyIndicator
                BusyIndicator {
                    anchors.centerIn: parent
                    running: asyncLoader.status === Loader.Loading
                    visible: running
                }

                sourceComponent: loadedComponent

                onLoaded: {
                    loadStatusText.text = qsTr("Component loaded asynchronously.");
                }
            }

            Column {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 20
                spacing: 16
                width: parent.width * 0.6

                Label {
                    text: qsTr("Loader Async Demo")
                    font.pixelSize: 20
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: qsTr("Loader with asynchronous: true loads components " +
                               "in a background thread, keeping UI responsive.")
                    wrapMode: Text.WordWrap
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }

                Row {
                    spacing: 12
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: qsTr("Load Component")
                        onClicked: {
                            loadStatusText.text = qsTr("Loading...");
                            asyncLoader.active = true;
                        }
                    }

                    Button {
                        text: qsTr("Unload Component")
                        onClicked: {
                            asyncLoader.active = false;
                            loadStatusText.text = qsTr("Component unloaded.");
                        }
                    }
                }

                Label {
                    id: loadStatusText
                    text: qsTr("Press Load to begin.")
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                    wrapMode: Text.WordWrap
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            /// 被异步加载的组件 —— 模拟一个较重的 UI 元素
            Component {
                id: loadedComponent

                Rectangle {
                    color: "#f0f0f0"
                    radius: 8
                    border.color: "#cccccc"
                    border.width: 1

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Label {
                            text: qsTr("Asynchronously Loaded Component")
                            font.pixelSize: 16
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Label {
                            text: qsTr("This component was loaded in a background thread.")
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Grid {
                            anchors.horizontalCenter: parent.horizontalCenter
                            columns: 4
                            spacing: 8

                            // 模拟一组子控件
                            Repeater {
                                model: 16
                                delegate: Rectangle {
                                    width: 50
                                    height: 50
                                    color: Qt.rgba(Math.random(),
                                                   Math.random(),
                                                   Math.random(), 1)
                                    radius: 4
                                }
                            }
                        }
                    }
                }
            }
        }

        // ---- Tab 3: XMLHttpRequest 网络请求 ----
        Item {
            Column {
                anchors.centerIn: parent
                spacing: 16
                width: parent.width * 0.7

                Label {
                    text: qsTr("XMLHttpRequest Demo")
                    font.pixelSize: 20
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: qsTr("Demonstrates XMLHttpRequest with the " +
                               "onreadystatechange pattern in QML.")
                    wrapMode: Text.WordWrap
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }

                Row {
                    spacing: 12
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: qsTr("GET httpbin.org/json")
                        onClicked: {
                            httpStatusText.text = qsTr("Sending request...");
                            responseData.text = "";
                            sendHttpRequest();
                        }
                    }

                    Button {
                        text: qsTr("GET httpbin.org/uuid")
                        onClicked: {
                            httpStatusText.text = qsTr("Sending request...");
                            responseData.text = "";
                            sendHttpRequest("https://httpbin.org/uuid");
                        }
                    }
                }

                Label {
                    id: httpStatusText
                    text: qsTr("Press a button to send a request.")
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                    wrapMode: Text.WordWrap
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }

                ScrollView {
                    width: parent.width
                    height: 200
                    clip: true

                    TextArea {
                        id: responseData
                        readOnly: true
                        wrapMode: TextEdit.Wrap
                        font.family: "monospace"
                        placeholderText: qsTr("Response data will appear here...")
                    }
                }
            }

            /// @brief 发送 XMLHttpRequest GET 请求。
            /// @param url 请求地址，默认为 httpbin.org/json。
            /// @note QML 中的 XMLHttpRequest 遵循 W3C 规范子集，
            ///       使用 onreadystatechange 回调模式。
            function sendHttpRequest(url) {
                if (!url) {
                    url = "https://httpbin.org/json";
                }
                var xhr = new XMLHttpRequest();
                xhr.onreadystatechange = function() {
                    if (xhr.readyState === XMLHttpRequest.HEADERS_RECEIVED) {
                        httpStatusText.text = qsTr("Headers received...");
                    } else if (xhr.readyState === XMLHttpRequest.DONE) {
                        if (xhr.status === 200) {
                            httpStatusText.text = qsTr("Request completed (HTTP %1)")
                                .arg(xhr.status);
                            // 尝试格式化 JSON，失败则直接显示原始文本
                            try {
                                var obj = JSON.parse(xhr.responseText);
                                responseData.text = JSON.stringify(obj, null, 2);
                            } catch (e) {
                                responseData.text = xhr.responseText;
                            }
                        } else {
                            httpStatusText.text = qsTr("Request failed (HTTP %1)")
                                .arg(xhr.status);
                            responseData.text = xhr.responseText || qsTr("(no response body)");
                        }
                    }
                };
                xhr.open("GET", url);
                xhr.send();
            }
        }
    }

    // 同步 TabBar 与 SwipeView 的当前索引
    tabBar.onCurrentIndexChanged: swipeView.currentIndex = tabBar.currentIndex
}
