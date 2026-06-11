// QtQuick 2.15 提供基础类型：Item、Rectangle、MouseArea 等
import QtQuick 2.15
// QtQuick.Controls 2.15 提供按钮、滑块等标准控件
import QtQuick.Controls 2.15

// 顶层窗口，作为整个示例的容器
ApplicationWindow {
    id: root
    title: "QML Syntax Advanced"
    width: 800
    height: 500
    visible: true

    // ================================================================
    // Section 4: Component.onCompleted 演示
    // ================================================================
    // @note Component.onCompleted 在组件实例化完毕后触发一次，
    //       适合做一次性初始化工作（如设置初始焦点、打印调试信息）。
    Component.onCompleted: {
        console.log("[Component.onCompleted] main.qml loaded successfully.")
        console.log("[Component.onCompleted] Window size:",
                    root.width, "x", root.height)
    }

    // 全局只读属性：一旦声明就不可在 QML 或 JS 中修改
    // @note readonly property 只能在声明时赋值，之后任何赋值都会报错。
    readonly property int maxValue: 100

    // 滑块值，用于驱动绑定演示中的矩形宽度
    property int sliderValue: 50

    // ================================================================
    // 主布局：使用 Column 将四个演示区块纵向排列
    // ================================================================
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        // ============================================================
        // Section 1: 绑定断裂（Binding Breakage）演示
        // ============================================================
        // QML 的属性绑定是响应式的基础，但 imperative 赋值会
        // 永久破坏绑定，之后属性将不再跟随源更新。
        Frame {
            width: parent.width
            height: 180

            Label {
                text: "Section 1: Binding Breakage"
                font.bold: true
                font.pixelSize: 16
            }

            Row {
                x: 10
                y: 30
                spacing: 30

                // --- 左侧：会断裂的绑定 ---
                Column {
                    spacing: 6

                    Label {
                        text: "Broken binding (imperative assignment)"
                        color: "red"
                    }

                    Rectangle {
                        id: brokenRect
                        // @warning 初始时有绑定：width 跟随 sliderValue 变化
                        // 但按钮的 onClicked 中会执行 imperative 赋值，
                        // 一旦赋值，此绑定就永久断裂。
                        width: sliderValue * 2
                        height: 40
                        color: "lightcoral"
                        radius: 4

                        Label {
                            anchors.centerIn: parent
                            text: "width = " + brokenRect.width.toFixed(0)
                        }
                    }

                    Button {
                        text: "Break binding (set width = 150)"
                        onClicked: {
                            // imperative 赋值：这会破坏 width 上的属性绑定
                            brokenRect.width = 150
                            statusLabel.text =
                                "Binding broken! Slider no longer affects red rect."
                        }
                    }
                }

                // --- 右侧：安全的绑定（使用辅助属性） ---
                Column {
                    spacing: 6

                    Label {
                        text: "Correct approach (helper property)"
                        color: "green"
                    }

                    Rectangle {
                        id: safeRect
                        // 使用辅助属性 rectWidth，按钮修改的是中间变量
                        // 而非直接修改绑定的目标属性
                        property int rectWidth: sliderValue * 2
                        width: rectWidth
                        height: 40
                        color: "lightgreen"
                        radius: 4

                        Label {
                            anchors.centerIn: parent
                            text: "width = " + safeRect.width.toFixed(0)
                        }
                    }

                    Button {
                        text: "Update via helper (rectWidth = 150)"
                        onClicked: {
                            // 修改辅助属性，不会破坏 width 上的绑定
                            safeRect.rectWidth = 150
                            statusLabel.text =
                                "Helper updated. Slider can still override green rect."
                        }
                    }
                }
            }

            // 共享滑块，驱动两个矩形的宽度绑定
            Slider {
                x: 10
                y: 135
                width: parent.width - 20
                from: 10
                to: root.maxValue   // 使用 readonly property
                value: root.sliderValue
                onMoved: root.sliderValue = value
            }
        }

        // 状态提示条
        Label {
            id: statusLabel
            width: parent.width
            text: "Drag the slider above and click buttons to observe."
            font.italic: true
            color: "gray"
            wrapMode: Text.WordWrap
        }

        // ============================================================
        // Section 2: required property 演示
        // ============================================================
        // required property 要求使用方在实例化时必须传入，
        // 否则 QML 引擎会报错，避免组件在缺少关键数据时运行。
        Frame {
            width: parent.width
            height: 100

            Label {
                text: "Section 2: required property"
                font.bold: true
                font.pixelSize: 16
            }

            // 实例化带有 required property 的组件
            GreetingItem {
                x: 10
                y: 35
                // @note greeting 是 required property，不传则 QML 引擎报错
                greeting: "Hello from required property!"
            }
        }

        // ============================================================
        // Section 3: readonly property 演示
        // ============================================================
        Frame {
            width: parent.width
            height: 80

            Label {
                text: "Section 3: readonly property (maxValue = "
                      + root.maxValue + ")"
                font.bold: true
                font.pixelSize: 16
            }

            Label {
                x: 10
                y: 35
                text: "root.maxValue is declared as `readonly property int maxValue: 100`. "
                      + "Any attempt to reassign it will cause a QML error."
                wrapMode: Text.WordWrap
                width: parent.width - 20
            }
        }
    }

    // ================================================================
    // 内联组件：演示 required property
    // ================================================================
    // @note 组件使用 required property 声明时，实例化该组件
    //       的地方必须提供该属性的值，否则 QML 引擎在加载时
    //       就会输出错误信息，防止运行时出现未定义行为。
    component GreetingItem: Rectangle {
        // 必须由父组件传入，缺少则 QML 加载时报错
        required property string greeting

        width: 400
        height: 50
        color: "lightyellow"
        radius: 6
        border.color: "goldenrod"

        Label {
            anchors.centerIn: parent
            text: parent.greeting
            font.pixelSize: 14
        }

        Component.onCompleted: {
            console.log("[GreetingItem] Created with greeting:",
                        greeting)
        }
    }
}
