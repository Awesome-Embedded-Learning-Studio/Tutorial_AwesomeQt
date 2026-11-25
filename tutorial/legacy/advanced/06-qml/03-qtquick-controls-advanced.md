---
title: "6.3 Qt Quick Controls 进阶：自定义样式"
description: "入门篇我们把 Qt Quick Controls 的基本控件用了一遍——Button、TextField、Slider 等，跑在默认样式下。说实话，默认样式做 Demo 够用了，但任何商业应用都需要自己的视觉风格。Qt Quick Controls 支持三种内置样式（Material、Fusion、Universal）和完全自定义的 Control 模板。"
---

# 现代Qt开发教程（进阶篇）6.3——Qt Quick Controls 进阶：自定义样式

## 1. 前言

入门篇我们把 Qt Quick Controls 的基本控件用了一遍——Button、TextField、Slider 等，跑在默认样式下。说实话，默认样式做 Demo 够用了，但任何商业应用都需要自己的视觉风格。Qt Quick Controls 支持三种内置样式（Material、Fusion、Universal）和完全自定义的 Control 模板。

这篇我们把内置样式切换、自定义 Control 模板、Palette 主题色管理、以及 ToolTip 全局配置这四个样式相关能力拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 Qt Quick Controls 模块。内置样式需要对应的 Qt Quick Controls 样式插件（Material 需要 Qt6::QuickControls2Material 等）。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS QuickControls2)` 引入。

## 3. 核心概念讲解

### 3.1 内置样式切换

Qt Quick Controls 提供 Basic（默认）、Fusion（桌面风格）、Material（Android 风格）、Universal（Windows/UWP 风格）四种内置样式。切换方式有两种。

通过环境变量（推荐，不需要改代码）：

```bash
# 运行时切换
QT_QUICK_CONTROLS_STYLE=Material ./myapp
```

通过 C++ 代码（在 QQmlApplicationEngine 加载 QML 之前）：

```cpp
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Material");
    QQmlApplicationEngine engine;
    engine.load(QUrl("main.qml"));
    return app.exec();
}
```

Material 样式支持主题和强调色配置：

```qml
// main.qml
import QtQuick.Controls.Material

ApplicationWindow {
    Material.theme: Material.Dark
    Material.accent: Material.Teal
    Material.primary: Material.BlueGrey
    Material.background: "#1a1a2e"
}
```

这些设置会自动传播到所有子控件——你不需要逐个 Button 设置颜色。

### 3.2 自定义 Control 模板——background / contentItem / indicator

如果内置样式都不满足需求，你可以通过重写 Control 的三个模板属性来自定义外观：`background`（背景）、`contentItem`（内容区域）、`indicator`（指示器，如复选框的勾选标记）。

```qml
// CustomButton.qml
import QtQuick.Controls

Button {
    id: control

    // 自定义背景
    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 40
        radius: 20
        color: control.pressed ? "#cc5500"
               : control.hovered ? "#ff7700"
               : "#ff9900"
        border.color: control.activeFocus ? "#ffffff" : "transparent"
        border.width: 2

        // 点击动画
        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    // 自定义内容
    contentItem: Text {
        text: control.text
        font.pixelSize: 16
        font.bold: true
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
```

`control.pressed`、`control.hovered`、`control.activeFocus` 是 Button 基类提供的状态属性，你可以在模板中自由引用它们来定义不同状态的视觉表现。

自定义 CheckBox 的 indicator：

```qml
CheckBox {
    id: control
    text: "Enable feature"

    indicator: Rectangle {
        implicitWidth: 26
        implicitHeight: 26
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 4
        border.color: control.checked ? "#4CAF50" : "#999999"

        Rectangle {
            width: 14
            height: 14
            x: 6
            y: 6
            radius: 3
            color: "#4CAF50"
            visible: control.checked
        }
    }
}
```

### 3.3 Palette 主题色统一管理

`Palette` 类型提供了一套统一的颜色定义，所有 Qt Quick Controls 内置控件都会自动使用 Palette 中的颜色。你可以在 ApplicationWindow 层面修改 Palette，所有子控件自动继承。

```qml
ApplicationWindow {
    palette.window: "#2b2b2b"
    palette.windowText: "#e0e0e0"
    palette.base: "#3c3c3c"
    palette.alternateBase: "#454545"
    palette.text: "#e0e0e0"
    palette.button: "#3c3c3c"
    palette.buttonText: "#e0e0e0"
    palette.highlight: "#4a9eff"
    palette.highlightedText: "#ffffff"
}
```

Palette 的优势是「一处设置，全局生效」——你不需要知道内部控件用了哪些颜色角色，只要把 Palette 设对了，整个应用的色调就统一了。

### 3.4 ToolTip 全局配置

`ToolTip` 是 Qt Quick Controls 的全局提示组件。你不需要给每个控件单独创建 ToolTip——通过附加属性 `ToolTip.text` 和 `ToolTip.visible` 就能配置：

```qml
Button {
    text: "Save"
    ToolTip.text: "Save changes to file (Ctrl+S)"
    ToolTip.visible: hovered
    ToolTip.delay: 500  // 悬停 500ms 后显示
    ToolTip.timeout: 3000  // 3 秒后自动隐藏
}
```

## 4. 踩坑预防

自定义 `background` 和 `contentItem` 时要注意 `implicitWidth` / `implicitHeight` 的设置。如果缺少这两个属性，Control 无法计算自身的首选大小，在布局中可能出现大小为 0 的情况。始终给 background 设置合理的 implicitWidth/implicitHeight。

## 5. 练习项目

练习项目：暗色主题 UI 套件。基于 Material Dark 样式，自定义 Button（圆角渐变背景）、TextField（底部线条样式）、Switch（自定义轨道和滑块）。用 Palette 统一管理应用色调。每个控件配上 ToolTip 说明。把三个控件放在 ColumnLayout 中展示。

## 6. 官方文档参考链接

[Qt 文档 · Qt Quick Controls](https://doc.qt.io/qt-6/qtquickcontrols-index.html) -- Controls 模块总览

[Qt 文档 · Customizing Controls](https://doc.qt.io/qt-6/qtquickcontrols-customize.html) -- 自定义控件模板指南

[Qt 文档 · Palette QML Type](https://doc.qt.io/qt-6/qml-qtquick-controls-palette.html) -- 主题色定义

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。样式切换、自定义模板、Palette 主题色、ToolTip——有了这四样，你的 Qt Quick Controls 应用就能拥有独特的视觉风格了。
