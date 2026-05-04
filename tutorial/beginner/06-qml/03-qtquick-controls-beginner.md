# 现代Qt开发教程（新手篇）6.3——Qt Quick Controls 组件基础

## 1. 从零开始认识 Qt Quick Controls

前两篇文章我们一直在用 `Rectangle`、`Text`、`MouseArea` 这些基础元素手搓 UI。这种方式用来学习 QML 的底层机制是不错的，但做实际项目就太痛苦了——一个按钮要自己画 `Rectangle`，加 `MouseArea` 处理点击，再写 `onPressed` 改颜色做视觉反馈，光一个能看的按钮就几十行代码。如果要做 ComboBox、Slider 这些更复杂的控件，从零手写的代价根本无法接受。

Qt Quick Controls 就是 Qt 官方提供的「成品控件库」。它封装了 `Button`、`TextField`、`ComboBox`、`CheckBox`、`Slider`、`SpinBox` 等一系列标准 UI 控件，每个控件都自带完整的视觉样式、交互反馈和无障碍支持。你只需要像使用普通 QML 类型一样声明它们，然后通过属性和信号处理器来定制行为就行了。

和 QtWidgets 的控件相比，Qt Quick Controls 的控件是完全基于 QML/Scene Graph 实现的，渲染路径更短、动画支持更好，而且天然支持自定义主题和样式。从 Qt 6 开始，Qt Quick Controls 的默认风格是 "Basic"（一种扁平化设计），你也可以切换为 "Fusion"、"Material"、"macOS"、"Windows" 等风格——只需要在 C++ 端设置一行代码或者在环境变量中指定。

---

## 2. 环境说明

本文档基于以下环境编写和验证：

- Qt 6.9.1，使用 `qt_add_executable` + `qt_add_qml_module` 构建
- CMake 3.26+，C++17 标准
- 需要链接 `Qt6::Quick` 和 `Qt6::QuickControls2` 两个模块
- QML 导入需要 `import QtQuick` 和 `import QtQuick.Controls`
- 布局需要 `import QtQuick.Layouts`

CMakeLists.txt 相比前两篇多了一个依赖：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)
```

以及链接时多了一个模块：

```cmake
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::QuickControls2
)
```

---

## 3. ApplicationWindow——QML 应用的主窗口

和前两篇用的 `Window` 不同，Qt Quick Controls 提供了一个更强大的 `ApplicationWindow`。它不仅具备 `Window` 的所有功能（宽度、高度、标题、可见性），还额外提供了 `menuBar`、`header`、`footer`、`ToolBar` 等标准窗口区域的内置支持。

```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "Qt Quick Controls Demo"

    // 菜单栏
    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem { text: "&New"; onTriggered: console.log("New") }
            MenuItem { text: "&Open"; onTriggered: console.log("Open") }
            MenuSeparator {}
            MenuItem { text: "&Quit"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "&Help"
            MenuItem { text: "&About"; onTriggered: console.log("About") }
        }
    }

    // 顶部工具栏
    header: ToolBar {
        Row {
            anchors.fill: parent
            spacing: 8

            Button {
                text: "Action 1"
                onClicked: console.log("Action 1")
            }
            Button {
                text: "Action 2"
                onClicked: console.log("Action 2")
            }
        }
    }

    // 底部状态栏（用 ToolBar 模拟）
    footer: ToolBar {
        Label {
            anchors.fill: parent
            anchors.leftMargin: 10
            text: "Ready"
            verticalAlignment: Text.AlignVCenter
        }
    }

    // 中央内容区——背景色
    background: Rectangle {
        color: "#f5f5f5"
    }
}
```

`ApplicationWindow` 把窗口分成了几个明确的区域：`menuBar` 在最顶部，`header` 紧跟菜单栏下方，中央是主内容区（由 `background` 定义背景），`footer` 在最底部。这种区域划分让你不需要手动计算各部分的布局位置，窗口大小变化时各区域会自动调整。

有一点要注意：`ApplicationWindow` 的内容区域（中央区域）不是通过 `children` 填充的，而是通过 `StackView`、`SwipeView` 或者直接在 `ApplicationWindow` 内部放置控件来填充。如果你直接在 `ApplicationWindow` 内放一个 `Rectangle` 并设置 `anchors.fill: parent`，它会填充 `header` 和 `footer` 之间的中央区域，而不是整个窗口——这一点和普通的 `Window` 不同。

---

## 4. 常用控件详解

### 4.1 Button——按钮

`Button` 是最基础的交互控件。它继承自 `AbstractButton`，自带 `text`、`icon`、`checkable`、`checked`、`pressed`、`highlighted`、`enabled` 等属性，以及 `onClicked` 信号。

```qml
Column {
    spacing: 12

    // 普通按钮
    Button {
        text: "Click Me"
        onClicked: console.log("Button clicked")
    }

    // 带图标的按钮
    Button {
        text: "Save"
        icon.name: "document-save"
        onClicked: console.log("Save clicked")
    }

    // 可切换按钮（checkable）
    Button {
        text: checked ? "Active" : "Inactive"
        checkable: true
        onCheckedChanged: console.log("Checked:", checked)
    }

    // 高亮按钮
    Button {
        text: "Important"
        highlighted: true
        onClicked: console.log("Important clicked")
    }

    // 禁用按钮
    Button {
        text: "Disabled"
        enabled: false
    }
}
```

`checkable: true` 把普通按钮变成了一个切换开关，点击后在选中/未选中之间切换。`highlighted: true` 会用主题的高亮色来渲染按钮，通常用于引导用户注意的关键操作。`icon.name` 使用系统主题图标名称（遵循 freedesktop.org 的图标命名规范），在不同平台上会自动匹配对应的图标。

### 4.2 TextField——文本输入框

`TextField` 是单行文本输入控件，对应 QtWidgets 的 `QLineEdit`。它支持占位文本、输入掩码、验证器、密码模式等常用功能。

```qml
Column {
    spacing: 12

    TextField {
        placeholderText: "Enter your name..."
        onTextChanged: console.log("Text:", text)
    }

    TextField {
        placeholderText: "Password"
        echoMode: TextInput.Password   // 密码模式
    }

    TextField {
        placeholderText: "Numbers only"
        // 限制只能输入数字
        validator: IntValidator { bottom: 0; top: 9999 }
    }

    TextField {
        placeholderText: "Max 10 characters"
        maximumLength: 10
    }
}
```

`echoMode` 属性控制文本的显示方式，`TextInput.Password` 会把输入的字符显示为圆点。`validator` 属性可以限制输入内容的格式——`IntValidator` 只允许整数，`DoubleValidator` 只允许浮点数，`RegularExpressionValidator` 则可以用正则表达式做任意格式的验证。

### 4.3 ComboBox——下拉选择框

`ComboBox` 是一个下拉列表选择控件。它有两种使用方式：静态列表（直接在 QML 中声明选项）和动态列表（通过 `ListModel` 或 JavaScript 数组提供数据）。

```qml
Column {
    spacing: 12

    // 静态列表
    ComboBox {
        model: ["Red", "Green", "Blue", "Yellow"]
        onCurrentIndexChanged: console.log("Selected:", currentText)
    }

    // 带 ListModel 的动态列表
    ComboBox {
        textRole: "name"        // 显示 model 中的 "name" 字段
        model: ListModel {
            ListElement { name: "Apple"; color: "#F44336" }
            ListElement { name: "Banana"; color: "#FFEB3B" }
            ListElement { name: "Grape"; color: "#9C27B0" }
        }
        onActivated: function(index) {
            console.log("Selected fruit:", currentText,
                       "Color:", model.get(index).color)
        }
    }

    // 可编辑的 ComboBox
    ComboBox {
        editable: true
        model: ["Option A", "Option B", "Option C"]
        onAccepted: function() {
            // 用户输入了自定义文本后按回车
            if (find(editText) === -1) {
                console.log("New item:", editText);
            }
        }
    }
}
```

`textRole` 属性在使用 `ListModel` 时特别有用——它指定了用 model 中的哪个字段作为显示文本。`editable: true` 让用户可以在输入框中输入自定义内容，而不只是从列表中选择。

### 4.4 CheckBox——复选框

`CheckBox` 提供一个可勾选的选项，适合布尔值输入。它支持三态模式（选中、未选中、部分选中），这在处理「全选/全不选」场景时非常实用。

```qml
Column {
    spacing: 12

    CheckBox {
        text: "Enable notifications"
        onCheckedChanged: console.log("Checked:", checked)
    }

    CheckBox {
        text: "Remember me"
        checked: true
    }

    // 三态复选框
    CheckBox {
        text: "Select All"
        tristate: true
        checkState: Qt.PartiallyChecked
        onCheckStateChanged: console.log("State:", checkState)
    }
}
```

三态模式下，`checkState` 的取值有三种：`Qt.Unchecked`（未选中，值为 0）、`Qt.PartiallyChecked`（部分选中，值为 1）、`Qt.Checked`（选中，值为 2）。普通模式下，`checked` 属性就是 `checkState === Qt.Checked` 的便捷方式。

### 4.5 Slider——滑块

`Slider` 提供一个可拖动的滑块控件，用于在一个范围内选择值。它有水平和垂直两种方向。

```qml
Column {
    spacing: 16

    // 水平滑块
    Slider {
        from: 0
        to: 100
        value: 50
        onValueChanged: console.log("Value:", value)
    }

    // 带步进的水平滑块
    Slider {
        from: 0
        to: 1.0
        value: 0.5
        stepSize: 0.1
        onValueChanged: console.log("Opacity:", value)
    }

    // 垂直滑块
    Row {
        spacing: 20

        Slider {
            orientation: Qt.Vertical
            from: 0
            to: 100
            value: 30
        }

        Slider {
            orientation: Qt.Vertical
            from: 0
            to: 100
            value: 70
        }
    }
}
```

`from` 和 `to` 定义了值的范围，`stepSize` 定义了每次移动的步进值。`value` 属性始终持有当前的滑块位置值。`Slider` 的 `onValueChanged` 信号在拖动过程中会频繁触发，所以如果你的响应逻辑比较重（比如触发网络请求），应该加上防抖处理。

---

## 5. QML 布局系统——ColumnLayout / RowLayout / GridLayout

前面的示例中我们用 `Column`、`Row` 和 `anchors` 来排列控件。这些虽然能用，但有一个明显的短板：它们不支持大小策略（size hint）。也就是说，当窗口大小变化时，`Column` 里的子元素不会自动调整大小——你只能通过手动绑定来控制每个元素的尺寸。

Qt Quick Layouts 提供了更强大的布局管理器：`ColumnLayout`（纵向排列）、`RowLayout`（横向排列）和 `GridLayout`（网格排列）。它们和 QtWidgets 的 `QVBoxLayout`、`QHBoxLayout`、`QGridLayout` 非常相似，支持 `Layout.fillWidth`、`Layout.fillHeight`、`Layout.preferredWidth`、`Layout.minimumWidth`、`Layout.alignment`、`Layout.margins` 等丰富的附加属性。

### 5.1 ColumnLayout——纵向布局

`ColumnLayout` 把子元素从上到下依次排列：

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 400
    height: 500
    visible: true
    title: "ColumnLayout Demo"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label { text: "Username:" }
        TextField {
            Layout.fillWidth: true
            placeholderText: "Enter username"
        }

        Label { text: "Password:" }
        TextField {
            Layout.fillWidth: true
            echoMode: TextInput.Password
            placeholderText: "Enter password"
        }

        CheckBox {
            text: "Remember me"
            Layout.alignment: Qt.AlignLeft
        }

        Button {
            text: "Login"
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            highlighted: true
            onClicked: console.log("Login clicked")
        }

        // 弹性空间——把后续元素推到底部
        Item { Layout.fillHeight: true }

        Label {
            text: "Forgot password?"
            Layout.alignment: Qt.AlignHCenter
            color: "#2196F3"
        }
    }
}
```

`Layout.fillWidth: true` 让控件横向填满布局的可用宽度。`Layout.preferredHeight: 44` 给按钮指定了推荐的 44 像素高度。`Item { Layout.fillHeight: true }` 是一个常见的技巧——它创建一个不可见的弹性空间，占据剩余的所有纵向空间，从而把后面的元素推到布局的底部。

### 5.2 RowLayout——横向布局

`RowLayout` 把子元素从左到右依次排列：

```qml
RowLayout {
    spacing: 12

    Label { text: "Search:" }
    TextField {
        Layout.fillWidth: true
        Layout.preferredWidth: 200
        placeholderText: "Type to search..."
    }
    Button {
        text: "Go"
        onClicked: console.log("Search")
    }
    ComboBox {
        model: ["All", "Documents", "Images", "Music"]
        Layout.preferredWidth: 120
    }
}
```

`Layout.preferredWidth` 设定了控件的推荐宽度，但不会阻止控件在空间不足时缩小。如果你希望某个控件有最小宽度保护，可以加上 `Layout.minimumWidth`。

### 5.3 GridLayout——网格布局

`GridLayout` 把子元素按行列网格排列，适合表单类的布局场景：

```qml
GridLayout {
    columns: 2
    rowSpacing: 12
    columnSpacing: 16

    Label { text: "First Name:" }
    TextField {
        Layout.fillWidth: true
        placeholderText: "John"
    }

    Label { text: "Last Name:" }
    TextField {
        Layout.fillWidth: true
        placeholderText: "Doe"
    }

    Label { text: "Email:" }
    TextField {
        Layout.fillWidth: true
        placeholderText: "john@example.com"
    }

    Label { text: "Country:" }
    ComboBox {
        Layout.fillWidth: true
        model: ["China", "USA", "Japan", "Germany", "UK"]
    }

    // 跨列按钮
    Button {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        Layout.preferredHeight: 44
        text: "Submit"
        highlighted: true
        onClicked: console.log("Submit clicked")
    }
}
```

`columns: 2` 指定网格为 2 列，子元素会自动按顺序填充到每个格子中。`Layout.columnSpan: 2` 让按钮横跨两列，这在表单底部的提交按钮上非常常见。`rowSpacing` 和 `columnSpacing` 分别控制行间距和列间距。

---

## 6. 弹出组件——Dialog / Popup / Menu

### 6.1 Dialog——标准对话框

`Dialog` 是一个模态弹出窗口，通常用于确认操作、显示信息或收集用户输入。Qt Quick Controls 提供了标准化的 `Dialog` 类型，带有头部、内容区和底部按钮区。

```qml
ApplicationWindow {
    id: window
    width: 600
    height: 400
    visible: true
    title: "Dialog Demo"

    Column {
        anchors.centerIn: parent
        spacing: 12

        Button {
            text: "Show Message Dialog"
            onClicked: messageDialog.open()
        }

        Button {
            text: "Show Confirm Dialog"
            onClicked: confirmDialog.open()
        }

        Button {
            text: "Show Input Dialog"
            onClicked: inputDialog.open()
        }
    }

    // 消息对话框
    Dialog {
        id: messageDialog
        title: "Information"
        modal: true
        anchors.centerIn: parent

        Label {
            text: "This is a message dialog.\nOperation completed successfully."
        }

        standardButtons: Dialog.Ok
        onAccepted: console.log("Message dialog closed")
    }

    // 确认对话框
    Dialog {
        id: confirmDialog
        title: "Confirm"
        modal: true
        anchors.centerIn: parent

        Label {
            text: "Are you sure you want to delete this item?\nThis action cannot be undone."
        }

        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: console.log("Confirmed")
        onRejected: console.log("Cancelled")
    }

    // 输入对话框
    Dialog {
        id: inputDialog
        title: "Enter Name"
        modal: true
        anchors.centerIn: parent

        Column {
            spacing: 8
            width: parent.width

            Label { text: "Please enter your name:" }
            TextField {
                id: nameField
                width: parent.width
                placeholderText: "Your name"
            }
        }

        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: console.log("Name:", nameField.text)
        onRejected: console.log("Input cancelled")
    }
}
```

`standardButtons` 属性是 `Dialog` 的便捷特性——你只需要用位或运算符（`|`）组合需要的标准按钮，Qt 会自动渲染并处理按钮点击。`Dialog.Ok`、`Dialog.Cancel`、`Dialog.Yes`、`Dialog.No`、`Dialog.Apply`、`Dialog.Close` 等都是预定义的标准按钮。`modal: true` 让对话框变为模态，用户必须先处理对话框才能继续操作主窗口。

### 6.2 Popup——通用弹出层

`Popup` 是更底层的弹出组件，它不自带标题栏和按钮，只提供弹出/关闭的基本行为和背景遮罩。它适合实现自定义的浮动面板、工具提示、通知栏等。

```qml
ApplicationWindow {
    id: window
    width: 600
    height: 400
    visible: true

    Button {
        anchors.centerIn: parent
        text: "Show Popup"
        onClicked: customPopup.open()
    }

    Popup {
        id: customPopup
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        width: 300
        height: 200
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#ffffff"
            radius: 12
            border.color: "#e0e0e0"
            border.width: 1

            // 阴影效果（简易版）
            Rectangle {
                anchors.fill: parent
                anchors.margins: -4
                radius: 14
                color: "transparent"
                border.color: "#10000000"
                z: -1
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            Text {
                text: "Custom Popup"
                font.pixelSize: 18
                font.bold: true
            }

            Text {
                text: "This is a custom popup without standard buttons.\nClick outside or press Escape to close."
                font.pixelSize: 14
                color: "#666666"
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Button {
                text: "Close"
                onClicked: customPopup.close()
            }
        }
    }
}
```

`closePolicy` 控制弹出的关闭方式。`Popup.CloseOnEscape` 允许按 Escape 键关闭，`Popup.CloseOnPressOutside` 允许点击弹出区域外部关闭。`x` 和 `y` 手动计算了居中位置——`Popup` 默认不会自动居中，需要你自己定位。

### 6.3 Menu——右键菜单与下拉菜单

`Menu` 用于显示一列可选的菜单项。它可以作为右键菜单使用，也可以附加到 `Button` 上作为下拉菜单。

```qml
ApplicationWindow {
    width: 600
    height: 400
    visible: true
    title: "Menu Demo"

    // 顶部菜单
    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem {
                text: "&New"
                onTriggered: console.log("New")
            }
            MenuItem {
                text: "&Open..."
                onTriggered: console.log("Open")
            }
            MenuSeparator {}
            MenuItem {
                text: "&Save"
                onTriggered: console.log("Save")
            }
            MenuItem {
                text: "Save &As..."
                onTriggered: console.log("Save As")
            }
            MenuSeparator {}
            MenuItem {
                text: "&Quit"
                onTriggered: Qt.quit()
            }
        }

        Menu {
            title: "&Edit"
            MenuItem {
                text: "&Undo"
                onTriggered: console.log("Undo")
            }
            MenuItem {
                text: "&Redo"
                onTriggered: console.log("Redo")
            }
            MenuSeparator {}
            MenuItem {
                text: "Cu&t"
                onTriggered: console.log("Cut")
            }
            MenuItem {
                text: "&Copy"
                onTriggered: console.log("Copy")
            }
            MenuItem {
                text: "&Paste"
                onTriggered: console.log("Paste")
            }
        }
    }

    // 右键菜单
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: function(mouse) {
            contextMenu.popup()
        }
    }

    Menu {
        id: contextMenu
        MenuItem {
            text: "Refresh"
            onTriggered: console.log("Refresh")
        }
        MenuItem {
            text: "Properties"
            onTriggered: console.log("Properties")
        }
        MenuSeparator {}
        MenuItem {
            text: "Help"
            onTriggered: console.log("Help")
        }
    }
}
```

`Menu` 中可以用 `MenuSeparator` 来添加分隔线，把功能相关的菜单项分组。`MenuItem` 的 `onTriggered` 信号在用户点击菜单项时触发。右键菜单的实现方式是在 `MouseArea` 的 `onClicked` 中判断 `acceptedButtons: Qt.RightButton`，然后调用 `menu.popup()` 来弹出菜单。

---

## 7. 完整示例

下面是一个综合演示 Qt Quick Controls 各种控件、布局和弹出组件的完整示例。它模拟了一个简易的「用户设置」面板，包含了本篇涉及的所有知识点。

项目结构：

```
03-qtquick-controls-beginner/
  CMakeLists.txt
  main.cpp
  Main.qml
```

**CMakeLists.txt**：

```cmake
# Qt 6 Qt Quick Controls 示例 CMake 配置
cmake_minimum_required(VERSION 3.26)
project(QuickControlsDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI QuickControlsDemo
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::QuickControls2
)
```

**main.cpp**：

```cpp
/*
 *  Qt 6 入门教程 - 示例 6.3
 *  主题：Qt Quick Controls 组件基础
 *
 * 本示例演示：
 * 1. ApplicationWindow 主窗口结构
 * 2. Button / TextField / ComboBox / CheckBox / Slider 常用控件
 * 3. ColumnLayout / RowLayout / GridLayout QML 布局
 * 4. Dialog / Popup / Menu 弹出组件
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/QuickControlsDemo/Main.qml"_qs);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
```

**Main.qml**：

```qml
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

        // 标题
        Label {
            Layout.columnSpan: 2
            text: "User Settings"
            font.pixelSize: 24
            font.bold: true
        }

        // 用户名
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

        // 语言选择
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

        // 音量滑块
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

        // 复选框组
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

        // 分隔线
        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            height: 1
            color: "#e0e0e0"
        }

        // 当前设置预览
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

        // 底部按钮区
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

        // 2 秒后自动关闭
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
```

编译运行后你会看到一个完整的设置面板界面。顶部有工具栏，包含主题选择和 About 按钮。中间是一个表单布局，包含文本输入、下拉选择、滑块、复选框等控件，右侧的预览区域会实时反映当前的设置状态。底部有 Reset 和 Apply 两个操作按钮——Reset 会弹出确认对话框，Apply 会弹出一个从底部滑出的成功提示并在 2 秒后自动消失。右键点击窗口空白区域还会弹出一个上下文菜单。

这个示例综合运用了 `ApplicationWindow` 的各个区域、`GridLayout` 的表单布局、六种常用控件、`Dialog` 的标准按钮、`Popup` 的自定义弹出，以及 `Menu` 的右键菜单和菜单栏。每一个组件都是通过声明式的方式使用的，没有一行手动的布局计算代码。

---

## 8. 小结

到这里我们已经把 Qt Quick Controls 的核心组件过了一遍。`ApplicationWindow` 提供了结构化的主窗口框架，`Button`、`TextField`、`ComboBox`、`CheckBox`、`Slider` 覆盖了最常见的交互控件需求，`ColumnLayout`、`RowLayout`、`GridLayout` 解决了控件排列和自适应大小的问题，`Dialog`、`Popup`、`Menu` 则提供了标准的弹出交互模式。这些组件组合在一起，已经足够搭建一个功能完整、交互合理的桌面应用界面了。

当然，Qt Quick Controls 提供的组件远不止这些——`SpinBox`、`Dial`、`TabBar`、`ToolBar`、`ScrollView`、`StackView`、`Drawer` 等等，每一个都有各自的使用场景。但掌握了本篇介绍的这些核心组件和布局之后，再学习其他组件只是查文档的事了。
