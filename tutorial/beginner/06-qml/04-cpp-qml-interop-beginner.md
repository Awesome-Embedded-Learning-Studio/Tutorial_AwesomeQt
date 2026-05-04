# 现代Qt开发教程（新手篇）6.4——C++ 与 QML 互操作基础

## 1. 为什么要让 C++ 和 QML 互相通信

前三篇教程里我们一直在 QML 的世界里打转——声明属性、绑定数据、摆控件、写布局。这些东西做纯展示型 UI 绰绰有余，但一个真实的桌面或嵌入式应用不可能只靠 QML 打天下。你的业务逻辑、数据模型、文件读写、网络请求、多线程计算这些重活，必须用 C++ 来扛。这就引出了 Qt Quick 开发中最核心的一座桥梁：C++ 与 QML 的互操作。

你可以把 QML 和 C++ 的关系理解成「前端和后端」。QML 负责呈现和交互，C++ 负责逻辑和数据。两者之间需要一条高效的双向通道：C++ 端要把数据暴露给 QML 显示，QML 端要把用户的操作反馈给 C++ 处理。Qt 提供了不止一条路径来完成这件事，但不同的路径适用场景不同，选择错了虽然功能上也能跑通，后期维护起来会非常痛苦。

这篇文章我们会逐一拆解 Qt 6 中最常用的四种互操作方式：用 `Q_PROPERTY` 把 C++ 属性暴露给 QML 的绑定系统、用 `QML_ELEMENT` 宏注册新的 C++ 类型让 QML 直接实例化、用 `QQmlContext::setContextProperty()` 把已有的 C++ 对象作为全局单例注入 QML 上下文、以及跨语言的方法调用和信号传递。每种方式我都会给出完整的可运行示例，同时讲清楚背后的机制和取舍。

---

## 2. 环境说明

本文档基于以下环境编写和验证：

- Qt 6.9.1，使用 `qt_add_executable` + `qt_add_qml_module` 构建
- CMake 3.26+，C++17 标准
- 需要链接 `Qt6::Quick` 模块
- 本篇示例涉及多个 C++ 头文件和 QML 文件，`qt_add_qml_module` 会自动处理资源打包和类型注册

CMakeLists.txt 中需要注意的配置包括 `CMAKE_AUTOMOC` 开启（处理 `Q_OBJECT` 宏的元对象编译），以及 `qt_add_qml_module` 中通过 `QML_FILES` 列出所有 QML 文件。对于使用 `QML_ELEMENT` 宏的类型注册方式，CMake 会通过 `qt_add_qml_module` 自动生成注册代码，不需要手动调用 `qmlRegisterType`。

---

## 3. Q_PROPERTY——把 C++ 属性接入 QML 绑定系统

### 3.1 Q_PROPERTY 是什么

`Q_PROPERTY` 是 Qt 元对象系统中的属性声明宏。一个被 `Q_PROPERTY` 修饰的类成员，会自动获得通知机制、读取/写入方法、以及与 QML 绑定系统的兼容性。当你在 C++ 类里写了 `Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)`，QML 引擎就能识别这个属性，把它当成一个普通的 QML 属性来使用——包括绑定追踪。

这一点非常关键。QML 的属性绑定之所以能自动追踪变化，底层依赖的就是 `NOTIFY` 信号。当 C++ 端通过 `setName("new value")` 修改了属性值并发射 `nameChanged()` 信号后，QML 引擎捕获到这个信号，就会自动更新所有绑定了该属性的 QML 元素。整个过程你不需要在 QML 里写任何监听代码，和纯 QML 属性的行为完全一致。

### 3.2 定义一个可暴露的 C++ 类

我们创建一个 `AppController` 类，它代表应用的核心状态控制器，包含用户名、计数器、主题颜色三个属性，以及一个 QML 可调用的方法。

```cpp
// app_controller.h
#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QQmlEngine>

class AppController : public QObject
{
    Q_OBJECT
    // 将 C++ 属性暴露给 QML 引擎
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY counterChanged)
    Q_PROPERTY(QString themeColor READ themeColor WRITE setThemeColor NOTIFY themeColorChanged)

    // 允许该类在 QML 中通过类型名直接实例化
    QML_ELEMENT

public:
    explicit AppController(QObject *parent = nullptr);

    // 属性读取方法
    QString userName() const;
    int counter() const;
    QString themeColor() const;

    // 属性写入方法（内部发射对应的 NOTIFY 信号）
    void setUserName(const QString &name);
    void setCounter(int value);
    void setThemeColor(const QString &color);

    // Q_INVOKABLE 使该方法可从 QML 中调用
    Q_INVOKABLE void increment();
    Q_INVOKABLE void reset();
    Q_INVOKABLE QString greeting() const;

signals:
    void userNameChanged();
    void counterChanged();
    void themeColorChanged();
    // 自定义信号也可以从 C++ 发射到 QML
    void notificationRequested(const QString &message);

private:
    QString m_userName;
    int m_counter = 0;
    QString m_themeColor = "#3498db";
};

#endif // APP_CONTROLLER_H
```

这个头文件有几个值得注意的地方。首先是三个 `Q_PROPERTY`，每个都声明了 `READ`、`WRITE` 和 `NOTIFY` 三件套。`READ` 指定读取方法，返回属性值；`WRITE` 指定写入方法，在内部发射变更信号；`NOTIFY` 指定一个无参信号，QML 引擎通过它来感知属性变化。如果缺了 `NOTIFY`，这个属性在 QML 里就变成了只读的一次性值——绑定追踪不会生效。

然后是 `QML_ELEMENT` 宏。这是 Qt 5.15 引入、Qt 6 大力推广的注册方式。过去我们需要手动调用 `qmlRegisterType<AppController>("MyModule", 1, 0, "AppController")`，现在只要在类声明里加上 `QML_ELEMENT`，配合 CMake 的 `qt_add_qml_module`，构建系统会自动完成类型注册。注册之后，QML 就能像使用内置类型一样使用 `AppController`。

最后看 `Q_INVOKABLE`。被这个宏修饰的成员函数，QML 引擎可以通过元对象系统调用它。参数和返回值会自动在 C++ 类型与 JavaScript 类型之间转换——`QString` 变成 JS 字符串，`int` 变成 JS 数字，`QVariantMap` 变成 JS 对象，以此类推。

### 3.3 实现文件

```cpp
// app_controller.cpp
#include "app_controller.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

QString AppController::userName() const
{
    return m_userName;
}

void AppController::setUserName(const QString &name)
{
    if (m_userName != name) {
        m_userName = name;
        emit userNameChanged();
    }
}

int AppController::counter() const
{
    return m_counter;
}

void AppController::setCounter(int value)
{
    if (m_counter != value) {
        m_counter = value;
        emit counterChanged();
    }
}

QString AppController::themeColor() const
{
    return m_themeColor;
}

void AppController::setThemeColor(const QString &color)
{
    if (m_themeColor != color) {
        m_themeColor = color;
        emit themeColorChanged();
    }
}

void AppController::increment()
{
    setCounter(m_counter + 1);
}

void AppController::reset()
{
    setCounter(0);
}

QString AppController::greeting() const
{
    if (m_userName.isEmpty()) {
        return "Hello, stranger!";
    }
    return "Hello, " + m_userName + "! Count: " + QString::number(m_counter);
}
```

每个 setter 内部都做了不等判断 `if (m_xxx != xxx)` 再发射信号。这不是防御性编程的多余操作，而是 Qt 属性系统的标准实践。如果不做这个判断，每次调用 setter 都会发射信号，哪怕值根本没变——这在 QML 端会导致不必要的绑定重算和 UI 刷新，严重时甚至造成无限循环。很多新手在这里踩坑，发现 UI 莫名其妙地闪烁或卡顿，最后追查到 setter 里缺少了这个守卫。

---

## 4. 注册方式对比——QML_ELEMENT vs setContextProperty

在把 C++ 对象送到 QML 之前，我们需要搞清楚两种主要的注册方式以及它们各自的适用场景。

### 4.1 QML_ELEMENT：类型注册，QML 端实例化

当你用 `QML_ELEMENT` 注册一个 C++ 类型后，QML 可以像使用原生类型一样直接声明这个类型的实例：

```qml
AppController {
    id: controller
    userName: "Charlie"
}
```

这种方式适合「每个 QML 实例对应一个独立的 C++ 对象」的场景。比如你有一个自定义的 `ImageViewer` 组件，每个实例都有自己的状态和配置，那就应该用类型注册。

在 CMake 端，`QML_ELEMENT` 配合 `qt_add_qml_module` 使用时，你需要把 C++ 源文件加到 `qt_add_executable` 中，构建系统会自动为你的 QML 模块生成类型注册信息。QML 中通过模块 URI 导入后就能使用这些类型。

### 4.2 setContextProperty：对象注入，C++ 端管理生命周期

`QQmlContext::setContextProperty()` 的思路完全不同。你不是注册一个「类型」，而是直接把一个已经构造好的 C++ 对象实例塞进 QML 的全局上下文。QML 里通过一个字符串名字来引用它，不需要 import 任何模块：

```cpp
QQmlApplicationEngine engine;
auto *controller = new AppController(&app);
engine.rootContext()->setContextProperty("appController", controller);
```

```qml
// QML 中直接用 appController 这个名字
Text {
    text: appController.userName
}
```

这种方式适合「全局唯一的后端对象」场景——比如应用级别的配置管理器、网络客户端、数据源。整个应用只需要一个实例，所有 QML 文件共享同一个对象。

两种方式不是互斥的。在实际项目中，全局性的后端服务用 `setContextProperty` 注入，而可复用的 UI 组件（每个实例有独立状态）用 `QML_ELEMENT` 注册，这是比较常见的组合。

---

## 5. 完整示例——双向通信演示

我们来看一个完整的项目，把前面讲的所有机制都串起来。这个示例包含一个 `AppController` 类（通过 `setContextProperty` 注入），QML 界面展示了 C++ 属性的实时绑定、QML 调用 C++ 方法、以及 C++ 发信号到 QML。

### 5.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.26)
project(CppQmlInteropDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    app_controller.h
    app_controller.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI CppQmlInteropDemo
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick
)
```

### 5.2 main.cpp

```cpp
/*
 *  Qt 6 入门教程 - 示例 6.4
 *  主题：C++ 与 QML 互操作基础
 *
 * 本示例演示：
 * 1. Q_PROPERTY 暴露 C++ 属性到 QML
 * 2. QQmlContext::setContextProperty() 注入对象
 * 3. QML 调用 Q_INVOKABLE 方法
 * 4. C++ 发射信号到 QML 的信号处理器
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include "app_controller.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 创建控制器实例，注入到 QML 上下文
    AppController controller;
    engine.rootContext()->setContextProperty("appController", &controller);

    const QUrl url(u"qrc:/CppQmlInteropDemo/Main.qml"_qs);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    // 定时从 C++ 端发射信号到 QML
    QTimer notificationTimer;
    notificationTimer.setInterval(5000);
    QObject::connect(&notificationTimer, &QTimer::timeout, &controller, [&controller]() {
        if (!controller.userName().isEmpty()) {
            emit controller.notificationRequested(
                "Hey " + controller.userName() + ", count is " + QString::number(controller.counter()));
        }
    });
    notificationTimer.start();

    return app.exec();
}
```

`main.cpp` 里我们做了两件关键的事。第一件是通过 `engine.rootContext()->setContextProperty("appController", &controller)` 把 `controller` 对象注入到 QML 的根上下文。注入之后，所有 QML 文件都能直接通过 `appController` 这个名字访问这个对象的属性和方法。注意我们传的是指针而不是对象副本——QML 引擎不会复制你的 C++ 对象，它只持有一个引用。

第二件是设置了一个 5 秒间隔的 `QTimer`，每次超时后通过 `emit controller.notificationRequested(...)` 发射一个自定义信号。这个信号在 QML 端通过 `onNotificationRequested` 处理器接收，用来演示「C++ 主动推送消息到 QML」这个能力。在实际项目中，这个信号可能来自网络回调、数据库变更通知、或者后台线程的计算结果。

### 5.3 Main.qml

```qml
// Main.qml — C++ 与 QML 互操作综合演示

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 640
    height: 520
    visible: true
    title: "C++ / QML Interop Demo"

    // --- 通知弹窗队列 ---
    property string notificationText: ""
    property bool showNotification: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // 标题
        Label {
            text: "C++ & QML Interop Demo"
            font.pixelSize: 24
            font.bold: true
        }

        // 用户名输入
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 16
            rowSpacing: 12

            Label {
                text: "User Name:"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
            TextField {
                id: nameInput
                Layout.fillWidth: true
                placeholderText: "Type your name..."
                // 双向绑定：QML 输入 -> C++ 属性
                onTextChanged: appController.userName = text
                Component.onCompleted: text = appController.userName
            }

            Label {
                text: "Counter:"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: appController.counter
                    font.pixelSize: 28
                    font.bold: true
                    color: "#2c3e50"
                    Layout.preferredWidth: 60
                    horizontalAlignment: Text.AlignHCenter
                }

                Button {
                    text: "+1"
                    onClicked: appController.increment()
                }
                Button {
                    text: "Reset"
                    onClicked: appController.reset()
                }
            }

            Label {
                text: "Theme:"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
            ComboBox {
                id: themeCombo
                Layout.fillWidth: true
                model: ["#3498db", "#e74c3c", "#2ecc71", "#f39c12", "#9b59b6"]
                displayText: "Pick a color"
                onActivated: function(index) {
                    appController.themeColor = model[index]
                }
            }
        }

        // 分隔线
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#ddd"
        }

        // Greeting 区域：展示 C++ 方法的返回值
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: greetingLabel.height + 24
            color: appController.themeColor
            radius: 8

            Label {
                id: greetingLabel
                anchors.centerIn: parent
                text: appController.greeting()
                font.pixelSize: 16
                font.bold: true
                color: "#ffffff"
            }

            // 颜色变化时的过渡动画
            Behavior on color {
                ColorAnimation { duration: 300 }
            }
        }

        // 状态面板
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: statusContent.height + 24
            color: "#f8f9fa"
            radius: 8
            border.color: "#dee2e6"

            Column {
                id: statusContent
                anchors.fill: parent
                anchors.margins: 12
                spacing: 4

                Text {
                    font.pixelSize: 13
                    text: "userName: " + appController.userName
                    color: "#495057"
                }
                Text {
                    font.pixelSize: 13
                    text: "counter: " + appController.counter
                    color: "#495057"
                }
                Text {
                    font.pixelSize: 13
                    text: "themeColor: " + appController.themeColor
                    color: "#495057"
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    // 接收 C++ 端发射的 notificationRequested 信号
    Connections {
        target: appController
        function onNotificationRequested(message) {
            notificationText = message
            showNotification = true
            notificationHideTimer.start()
        }
    }

    // 通知弹出
    Rectangle {
        id: notificationBar
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        width: notificationText.length * 9 + 40
        height: 40
        color: "#2c3e50"
        radius: 8
        opacity: showNotification ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 300 }
        }

        Text {
            anchors.centerIn: parent
            text: notificationText
            color: "#ffffff"
            font.pixelSize: 13
        }

        Timer {
            id: notificationHideTimer
            interval: 3000
            onTriggered: showNotification = false
        }
    }
}
```

这个 QML 文件把所有互操作机制都用上了。`appController.userName`、`appController.counter`、`appController.themeColor` 是从 C++ 端 `Q_PROPERTY` 暴露过来的属性，它们被绑定到各种 QML 元素的显示内容上。当 C++ 端修改这些属性的值时，QML 界面会自动更新。反过来，`onTextChanged: appController.userName = text` 这行代码把 QML 输入框的变化写回 C++ 属性，形成了双向数据流。

`appController.increment()` 和 `appController.reset()` 是通过 `Q_INVOKABLE` 暴露的 C++ 方法，QML 通过 JavaScript 表达式直接调用。`appController.greeting()` 更有意思——它是一个有返回值的方法，QML 把返回值直接当作属性绑定来使用。每次 `userName` 或 `counter` 变化时，QML 引擎会重新调用这个方法获取最新结果。

最下面的 `Connections` 块展示了「C++ 发信号到 QML」的用法。`target: appController` 指定了监听的对象，`function onNotificationRequested(message)` 是信号处理器——函数名遵循 `on` + 信号名（首字母大写）的命名规则。当 C++ 端通过 `emit notificationRequested(...)` 发射信号时，这个函数就会被调用，参数类型自动从 C++ 映射到 JavaScript。

---

## 6. 关于 setContextProperty 的注意事项

`setContextProperty` 使用起来简单粗暴，但它有几个容易踩的坑需要留意。

第一个坑是对象的生命周期。你传给 `setContextProperty` 的对象必须比 QML 引擎活得更久——至少一样久。如果 C++ 对象先于 QML 引擎销毁，QML 尝试访问一个悬空指针，程序直接崩溃。我们的示例里 `controller` 是 `main()` 函数的栈变量，它的生命周期覆盖了 `app.exec()` 的整个过程，所以没问题。但如果你的 C++ 对象是 `new` 出来的，记得选择合适的 `parent` 或者手动管理析构时机。

第二个坑是上下文的层级关系。QML 的上下文是树状结构的，子上下文可以访问父上下文中注入的属性，但反过来不行。`setContextProperty` 如果设置在根上下文上，所有 QML 组件都能访问；如果设置在某个组件的上下文上，只有该组件及其子组件能看到。全局性的东西放根上下文，局部性的东西放组件上下文，这个原则和全局变量/局部变量的道理一样。

第三个坑是性能。每次调用 `setContextProperty` 都会触发 QML 引擎重新评估所有使用该属性的绑定表达式。如果你在一个循环里频繁更新 context property，界面会变得非常卡。正确的做法是用 `Q_PROPERTY` + `NOTIFY` 信号，让引擎只在属性真正变化时更新。这也是为什么我们的 `AppController` 同时提供了 `Q_PROPERTY` 和 `setContextProperty`——前者负责精细的变更通知，后者负责对象注入，各司其职。

---

## 7. 小结

到这里我们走通了 C++ 与 QML 互操作的四种核心路径。`Q_PROPERTY` 是基础中的基础，它让你的 C++ 类属性能无缝接入 QML 的绑定系统，是所有互操作的前提。`QML_ELEMENT` 是现代 Qt 6 推荐的类型注册方式，省去了手写 `qmlRegisterType` 的样板代码。`setContextProperty` 适合注入全局唯一的后端对象，简单直接但需要注意生命周期管理。`Q_INVOKABLE` 和信号则打通了跨语言的方法调用和事件传递。

到这里就大功告成了。下一篇我们进入 QML 的动画与状态机，让 UI 动起来。
