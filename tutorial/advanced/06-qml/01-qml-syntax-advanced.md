---
title: "6.1 QML 语法进阶：绑定陷阱、required 属性、readonly"
description: "入门篇我们把 QML 的基本语法跑通了——属性声明、信号槽、组件定义。写个简单的 QML 页面确实够用了。但 QML 的属性绑定系统有一些让人头疼的陷阱——最经典的就是「绑定断裂」：你在某个事件处理里给属性赋了一个固定值，原来的绑定就永久丢失了，后续数据源变化时属性不再更新。"
---

# 现代Qt开发教程（进阶篇）6.1——QML 语法进阶：绑定陷阱、required 属性、readonly

## 1. 前言 / QML 的「暗坑」比语法多

入门篇我们把 QML 的基本语法跑通了——属性声明、信号槽、组件定义。写个简单的 QML 页面确实够用了。但 QML 的属性绑定系统有一些让人头疼的陷阱——最经典的就是「绑定断裂」：你在某个事件处理里给属性赋了一个固定值，原来的绑定就永久丢失了，后续数据源变化时属性不再更新。

更阴险的是，绑定断裂在开发阶段经常看不出来——因为开发时数据源不怎么变化。等部署到生产环境，数据持续变化，UI 却不跟着更新了。这时候你回头排查，发现某个 `onClicked` 里有一行 `width = 200`，它把原来的 `width: parent.width * 0.5` 绑定给覆盖了。

Qt 6 还引入了 `required property`——声明为 required 的属性必须在实例化时由父组件传值，否则 QML 引擎报错拒绝创建。这在组件 API 设计中非常有用——强制调用方提供必要的数据，不再有「忘了传参数导致组件空转」的问题。

这篇我们把绑定断裂的排查与修复、`required property` 的正确用法、`readonly property` 的设计意图、以及 `Component.onCompleted` 的延迟初始化这四个核心知识点拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 QML 语言和 Qt Quick 模块。所有示例都可以在 Qt Quick Controls 项目中直接使用。本篇涉及的概念适用于所有 QML 场景，不依赖特定的 QML 模块。

## 3. 核心概念讲解

### 3.1 绑定断裂——命令式赋值如何杀死绑定

QML 属性绑定的核心规则是：**一个属性同一时刻只能有一个绑定**。当你用声明式语法 `width: parent.width * 0.5` 创建绑定时，QML 引擎持续监听 `parent.width` 的变化并自动更新 `width`。但如果你在任何命令式代码中（事件处理、函数调用等）给 `width` 赋了一个固定值，原来的绑定就被永久替换了。

```qml
Rectangle {
    width: parent.width * 0.5  // 声明式绑定
    height: parent.height * 0.5
    color: "lightblue"

    MouseArea {
        anchors.fill: parent
        onClicked: {
            parent.width = 200  // 💀 绑定断裂！
            // 此后 parent.width 变化时，width 不再更新
        }
    }
}
```

这个问题的排查方式是在运行时留意控制台的 `"Binding loop detected"` 或 `"Cannot assign to read-only"` 警告。不过绑定断裂本身不会产生任何警告——它只是静默地停止更新。所以最好的策略是预防：

**规则一：永远不要在命令式代码中给有绑定的属性赋值。** 如果需要根据条件改变属性值，用 `Binding` 元素（下一篇详讲）或者用中间属性中转。

```qml
Rectangle {
    id: rect
    // 用中间属性控制是否固定宽度
    property bool useFixedWidth: false
    property int fixedWidth: 200

    width: useFixedWidth ? fixedWidth : parent.width * 0.5
    height: parent.height * 0.5

    MouseArea {
        anchors.fill: parent
        onClicked: {
            rect.useFixedWidth = true  // 安全：没有覆盖 width 的绑定
        }
    }
}
```

**规则二：如果确实需要命令式赋值，用 `Qt.binding()` 重建绑定。**

```qml
onClicked: {
    rect.width = Qt.binding(function() {
        return rect.parent.width * 0.3  // 新的绑定表达式
    });
}
```

`Qt.binding()` 接受一个 JavaScript 函数，返回一个新的绑定对象。赋值后 `width` 就有了新的绑定，后续 `parent.width` 变化时会按新公式更新。

现在有一道调试题。以下代码中，按钮点击后 `displayText` 会显示什么？连续点击两次后呢？

```qml
property int counter: 0
property string displayText: "Count: " + counter

Button {
    onClicked: {
        counter++
        displayText = "Clicked!"
    }
}
```

第一次点击后 `displayText` 显示 `"Clicked!"`——因为命令式赋值覆盖了绑定。第二次点击后仍然是 `"Clicked!"`——因为绑定已经断裂，`counter` 的变化不再影响 `displayText`。这就是绑定断裂的典型表现：第一次看起来正常，后续数据变化不再反映。

### 3.2 required property——强制传值的组件契约

Qt 6 引入的 `required property` 让组件可以声明「实例化时必须提供这个属性值」。如果父组件没有传值，QML 引擎在创建时直接报错。

```qml
// UserCard.qml
import QtQuick.Controls

Item {
    id: root
    required property string userName
    required property string userEmail
    required property int userId

    width: 300
    height: 80

    Text {
        text: root.userName
        font.bold: true
    }
    Text {
        y: 30
        text: root.userEmail
        color: "gray"
    }
}
```

使用时必须传值：

```qml
// 正确：传了所有 required 属性
UserCard {
    userName: "Alice"
    userEmail: "alice@example.com"
    userId: 42
}

// 错误：缺少 userName → QML 引擎报错
UserCard {
    userEmail: "bob@example.com"
    userId: 7
}
```

`required property` 的设计意图是组件 API 的「编译期检查」。在 C++ 中你用构造函数参数强制调用方提供必要数据，在 QML 中你用 `required property` 达到同样的效果。它比普通的 `property` 加默认值更安全——默认值可能掩盖了「调用方忘了传值」的问题，`required` 则让错误立刻暴露。

注意 `required` 不能和默认值同时使用——`required property string name: "default"` 是语法错误。required 就是「没有默认值，必须由外部提供」。

### 3.3 readonly property——组件内部计算的受保护输出

`readonly property` 声明的属性只能在声明时赋予一个绑定表达式，之后不能被命令式赋值覆盖。外部组件可以读取但不能写入。

```qml
Rectangle {
    id: root
    property int rawValue: 0

    // 只读的格式化输出，外部不能修改
    readonly property string formattedValue: {
        if (rawValue >= 1000000)
            return (rawValue / 1000000).toFixed(1) + "M"
        if (rawValue >= 1000)
            return (rawValue / 1000).toFixed(1) + "K"
        return rawValue.toString()
    }
}
```

外部使用时只能读：

```qml
Text {
    text: root.formattedValue  // ✅ 读取
    // root.formattedValue = "abc"  // ❌ 编译错误
}
```

`readonly` 的典型应用场景是组件的「派生属性」——由内部状态计算得出、对外暴露但不允许外部篡改的值。它也间接防止了绑定断裂——因为 readonly 属性不能被命令式赋值，所以它的绑定永远不会断裂。

### 3.4 Component.onCompleted——延迟初始化的正确姿势

`Component.onCompleted` 在组件创建完成时触发一次。它常用于那些需要在所有属性初始化完成后才执行的操作——比如根据初始属性值计算其他属性、启动初始动画、连接 C++ 信号。

但 `Component.onCompleted` 里给有绑定的属性赋值同样会断绑定。安全用法是只在这里做「一次性初始化」或者调用 C++ 方法。

```qml
Item {
    property var backend: null

    Component.onCompleted: {
        // 安全：给 null 属性赋值（没有绑定被覆盖）
        backend = createBackend()

        // 安全：调用 C++ 方法
        backend.initialize()

        // 危险：覆盖绑定的属性
        // width = 500  // 💀 会断裂绑定
    }
}
```

## 4. 踩坑预防

第一个坑是 `Qt.binding()` 中的闭包捕获。`Qt.binding(function() { return someObject.value })` 捕获的是 `someObject` 的引用。如果 `someObject` 被销毁了，绑定函数执行时访问的是 null，QML 会输出 `"TypeError: Cannot read property"` 警告，属性值变为 `undefined`。确保绑定函数中引用的对象生命周期足够长，或者用 `??` 运算符做防御：`return someObject?.value ?? 0`。

第二个坑是 `required property` 与 `Loader` 的交互。`Loader { sourceComponent: UserCard {} }` 异步加载组件时，如果 `UserCard` 有 required property 但 Loader 没有传值，组件创建会失败。`Loader` 不会自动把自身的属性传递给加载的组件——你需要在 `onLoaded` 回调里手动设置，或者用 `setSource` 传属性。

## 5. 练习项目

练习项目：可配置的仪表盘卡片组件。用 `required property` 声明标题和数据源（C++ QObject 指针）。`readonly property` 派生计算当前值和趋势箭头。组件内部用 `Binding` 元素（下一篇内容预告）处理主题切换时的样式变化。三张卡片排列在 RowLayout 中，每张卡片传不同的数据源。

完成标准：不传 required 属性时组件报错、readonly 属性值正确派生、数据源变化时卡片自动更新。

## 6. 官方文档参考链接

[Qt 文档 · QML Property Binding](https://doc.qt.io/qt-6/qtqml-syntax-propertybinding.html) -- 属性绑定系统完整说明，包含绑定断裂和重建

[Qt 文档 · QML Required Properties](https://doc.qt.io/qt-6/qtqml-cppintegration-exposecppattributes.html) -- required property 语法和组件契约

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。绑定断裂的预防、required 强制传值、readonly 派生属性、Component.onCompleted 的安全用法——这四个知识点搞定了，你的 QML 组件 API 设计能力就上了一个台阶。
