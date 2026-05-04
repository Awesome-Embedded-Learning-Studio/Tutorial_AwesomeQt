# 现代Qt开发教程（新手篇）5.13——Qt SCXML 状态图基础

## 1. 前言：从手写状态机到声明式状态图

上一篇我们用 Qt StateMachine 手工搭建了一个任务管理器的状态机——定义 QState、连接 addTransition、配置 assignProperty，代码量不小，但逻辑清晰。问题在于，当状态数量增长到十几个、转换路径变得错综复杂时，纯代码描述的状态机就开始变得难以阅读了。你需要在大段 C++ 代码里来回跳转才能看清"哪个状态连向哪里"，每次修改转换路径都得重新编译验证。

SCXML（State Chart XML）是 W3C 定义的一种状态图描述语言，本质上就是用 XML 格式来声明有限状态机。Qt 的 Scxml 模块实现了 SCXML 标准，让我们可以把状态机的结构写在独立的 .scxml 文件里，然后用 QScxmlStateMachine 直接加载运行。状态机的结构描述和业务逻辑代码彻底分离——状态图文件可以用可视化工具编辑、审查、生成文档，C++ 代码只负责响应状态变化和提交事件触发转换。

这篇我们要做的是从零编写一个 .scxml 状态图文件，用 QScxmlStateMachine 加载并驱动它，学会 submitEvent 提交事件触发状态转换，再看看如何把 SCXML 状态机塞进 QML 界面里直接用。最后我们会对比 SCXML 和 Qt StateMachine 的适用场景，搞清楚什么情况下该选哪个。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 Scxml 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Scxml Widgets)
```

Scxml 模块在 Qt Installer 中需要单独勾选。从源码编译的话它在 qtscxml 仓库中。部分发行版的包名是 qt6-scxml，Debian/Ubuntu 系列可以用 apt install qt6-scxml-dev 安装。

Scxml 模块的核心是 QScxmlStateMachine 类，它负责解析 .scxml 文件并驱动状态机运行。编译时 CMake 会通过 qt_add_executable 自动处理 .scxml 文件的解析和代码生成——实际上 Qt 内部会用一个叫 scxmlc 的编译器把 .scxml 编译成 C++ 代码，我们不需要手动操作这一步。

工具链方面和前面的教程一致：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+ 构建系统。

## 3. 核心概念讲解

### 3.1 SCXML 文件结构——状态机的 XML 描述

SCXML 文件的核心结构并不复杂，你可以把它理解为"用 XML 写的流程图"。根元素是 `<scxml>`，里面包含 `<state>` 节点表示各个状态，`<transition>` 标签表示状态之间的转换，`event` 属性指定触发转换的事件名。

下面是一个简单的交通灯状态图，包含红灯、绿灯、黄灯三个状态的循环转换：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<scxml xmlns="http://www.w3.org/2005/07/scxml"
       version="1.0"
       initial="red">

    <state id="red">
        <onentry>
            <log label="TrafficLight" expr="'进入红灯状态'"/>
        </onentry>
        <transition event="next" target="green"/>
    </state>

    <state id="green">
        <onentry>
            <log label="TrafficLight" expr="'进入绿灯状态'"/>
        </onentry>
        <transition event="next" target="yellow"/>
    </state>

    <state id="yellow">
        <onentry>
            <log label="TrafficLight" expr="'进入黄灯状态'"/>
        </onentry>
        <transition event="next" target="red"/>
    </state>

</scxml>
```

逐行拆一下这个文件。`<scxml>` 根元素的 `initial="red"` 指定了状态机的初始状态为 "red"。每个 `<state>` 的 `id` 属性是状态名称，在整个文档中必须唯一。`<transition>` 的 `event` 属性指定触发事件名（字符串），`target` 属性指定目标状态的 id。`<onentry>` 里的 `<log>` 标签会在状态进入时输出日志，`expr` 是表达式，单引号括起来表示字符串字面量。

这个状态图的逻辑非常直白：红灯收到 "next" 事件就转到绿灯，绿灯收到 "next" 事件就转到黄灯，黄灯收到 "next" 事件再回到红灯，形成循环。整个状态机的结构一目了然，不需要在代码中来回翻找。

SCXML 还支持很多高级特性，比如 `<parallel>` 并行状态、`<history>` 历史状态、`<datamodel>` 数据模型、`<invoke>` 调用外部服务等，不过这些在入门阶段用得不多，我们先把基础结构搞熟。

### 3.2 QScxmlStateMachine——加载和驱动状态机

有了 .scxml 文件之后，下一步就是在 C++ 代码中加载它。QScxmlStateMachine 是 Qt SCXML 模块的核心类，负责解析 .scxml 文件并创建运行时的状态机实例。

加载一个 .scxml 文件只需要两步：

```cpp
#include <QScxmlStateMachine>

// 从文件加载状态机
auto *machine = QScxmlStateMachine::fromFile(
    QStringLiteral(":/statemachine/trafficlight.scxml"));

if (!machine) {
    qWarning() << "加载 SCXML 文件失败";
    return -1;
}

// 启动状态机
machine->start();
```

QScxmlStateMachine::fromFile() 静态方法接受文件路径（支持 qrc 资源路径），返回一个 QScxmlStateMachine 指针。如果文件格式有误或者不符合 SCXML 规范，会返回 nullptr，所以加载后必须检查返回值。

状态机启动后自动进入 `<scxml>` 根元素的 `initial` 属性指定的初始状态。此时状态机就在事件循环中等待——和我们上一篇手写的 QStateMachine 行为一致，不消耗 CPU，只等待事件触发转换。

触发状态转换的核心方法是 submitEvent：

```cpp
// 提交 "next" 事件，触发当前状态的 <transition event="next">
machine->submitEvent(QStringLiteral("next"));
```

submitEvent 的参数是事件名（字符串），对应 .scxml 文件中 `<transition>` 的 `event` 属性。调用后状态机会检查当前状态是否有匹配该事件名的转换，如果有就执行转换到目标状态。

你也可以通过连接状态机的信号来监控状态变化：

```cpp
// 监控状态机进入指定状态
connect(machine, &QScxmlStateMachine::entered,
        [](const QSet<QString> &states) {
    qDebug() << "当前活跃状态:" << states;
});
```

entered 信号在每次状态转换后触发，参数是一个 QSet<QString>，包含了当前所有活跃的状态名（并行状态下会有多个同时活跃的状态）。对于简单的非并行状态机，这个集合通常只有一个元素。

### 3.3 数据模型——SCXML 内部的变量系统

SCXML 规范定义了一个叫 `<datamodel>` 的数据模型机制，允许在 .scxml 文件内部定义变量、读写变量、在转换条件中引用变量。这在需要在状态转换中携带数据的场景下非常有用。

```xml
<scxml xmlns="http://www.w3.org/2005/07/scxml"
       version="1.0"
       initial="idle"
       datamodel="ecmascript">

    <datamodel>
        <data id="counter" expr="0"/>
    </datamodel>

    <state id="idle">
        <transition event="increment" target="counting">
            <assign location="counter" expr="counter + 1"/>
        </transition>
    </state>

    <state id="counting">
        <transition event="check" cond="counter >= 5" target="done"/>
        <transition event="check" cond="counter < 5" target="idle"/>
    </state>

    <state id="done">
        <transition event="reset" target="idle">
            <assign location="counter" expr="0"/>
        </transition>
    </state>

</scxml>
```

`<datamodel>` 的 `datamodel="ecmascript"` 属性指定使用 ECMAScript 表达式求值器（Qt SCXML 支持的两种数据模型之一，另一种是 "null" 即无数据模型）。`<data>` 定义变量，`id` 是变量名，`expr` 是初始值表达式。`<assign>` 在转换时修改变量值，`cond` 属性在 `<transition>` 上添加条件守卫——只有条件为真时转换才会执行。

在 C++ 代码中，你可以通过 QScxmlStateMachine 的方法读写这些变量：

```cpp
// 读取 SCXML 数据模型中的变量
QVariant counter = machine->dataModel()->value(
    QStringLiteral("counter"));

// 写入变量
machine->dataModel()->setValue(
    QStringLiteral("counter"), 10);
```

不过说实话，对于大多数 Qt 应用来说，直接在 C++ 侧管理业务数据、在 SCXML 侧只管状态结构，是更清晰的做法。SCXML 数据模型适合那些需要跨平台复用同一个状态图文件的场景——比如状态图要同时被 C++ 后端和 Web 前端使用，数据放在状态图内部可以保持逻辑自包含。

### 3.4 与 QML 集成——在 QML 中驱动 SCXML 状态机

Qt SCXML 提供了专门的 QML 集成支持。CMake 中用 qt6_add_statecharts 宏把 .scxml 文件编译成 QML 可用的类型，然后在 QML 里直接实例化状态机、绑定状态属性、提交事件。

CMake 配置需要加上 QML 相关模块和状态图编译步骤：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Scxml Qml Quick)

qt6_add_statecharts(STATES
    statemachine/trafficlight.scxml)
```

qt6_add_statecharts 会调用 scxmlc 编译器处理 .scxml 文件，生成的 C++ 代码会包含一个可直接在 QML 中使用的状态机类型。

在 QML 中使用时，状态机的当前活跃状态可以通过字符串匹配来绑定属性：

```qml
import QtScxml 1.0 as Scxml

Window {
    id: root
    visible: true

    // 实例化状态机（由 qt6_add_statecharts 生成）
    TrafficLight {
        id: stateMachine
        running: true
    }

    Rectangle {
        anchors.fill: parent

        // 根据当前状态绑定颜色
        color: {
            if (stateMachine.isActive("red"))   return "red"
            if (stateMachine.isActive("green")) return "green"
            if (stateMachine.isActive("yellow")) return "yellow"
            return "gray"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: stateMachine.submitEvent("next")
        }
    }
}
```

isActive(stateName) 方法检查指定状态是否在当前活跃状态集合中。submitEvent(event) 从 QML 侧提交事件触发转换。这种 QML 集成方式的优势在于状态变化可以直接驱动 UI 属性，省去了 C++ 中间层的信号转发。

不过要注意一点：qt6_add_statecharts 生成的 QML 类型名是根据 .scxml 文件名自动推导的。比如 trafficlight.scxml 会生成名为 TrafficLight 的 QML 类型（首字母大写、驼峰化）。如果你需要自定义类型名或者导出到特定的 QML import 路径，需要在 CMake 中额外配置。

### 3.5 SCXML vs Qt StateMachine——怎么选

这两套方案都能实现有限状态机，选择的标准主要取决于项目需求。

Qt StateMachine（上一篇学的那个）的优势在于完全在 C++ 代码中定义状态和转换，类型安全，和 Qt 信号/槽系统深度集成，调试方便（断点直接打在状态构造代码上），不需要额外的文件和编译步骤。它的劣势是状态多了之后代码可读性下降——你需要在大段 C++ 代码里找状态定义和转换关系，没有全局视图。

SCXML 的优势在于状态结构独立于 C++ 代码，声明式的 XML 格式天然适合可视化工具编辑（Qt Creator 就有 SCXML 可视化编辑器），状态图文件可以被多个前端复用（C++ / QML / 甚至其他语言实现的 SCXML 引擎），状态机结构可以通过审查 XML 文件来验证而不用读代码。它的劣势是引入了额外的编译步骤（scxmlc 代码生成），调试时需要跨 XML 和 C++ 两个文件来回跳转，数据模型的 ECMAScript 表达式求值在性能敏感场景可能有开销。

实际项目中，如果你的状态机相对简单（十个状态以内）且只在 C++ 侧使用，Qt StateMachine 就够用。如果状态机结构复杂需要可视化编辑、需要跨 QML/C++ 共享、或者团队里有专门负责状态逻辑设计的人（他可能不写 C++），SCXML 是更好的选择。两者不冲突——甚至可以在同一个项目中混用。

## 4. 综合示例：SCXML 驱动的交通灯模拟器

把前面的知识串起来，我们做一个 SCXML 驱动的交通灯模拟器。状态图包含红灯、绿灯、黄灯三个状态，通过按钮提交 "next" 事件循环切换，界面用 QWidget 显示当前灯色和状态名。同时演示带事件数据的 submitEvent 用法——提交事件时携带持续时间参数。

首先是 .scxml 状态图文件，命名为 trafficlight.scxml：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<scxml xmlns="http://www.w3.org/2005/07/scxml"
       version="1.0"
       initial="red"
       name="TrafficLight">

    <state id="red">
        <onentry>
            <log label="TrafficLight" expr="'红灯亮起 - 停车'"/>
        </onentry>
        <transition event="next" target="green"/>
    </state>

    <state id="green">
        <onentry>
            <log label="TrafficLight" expr="'绿灯亮起 - 通行'"/>
        </onentry>
        <transition event="next" target="yellow"/>
    </state>

    <state id="yellow">
        <onentry>
            <log label="TrafficLight" expr="'黄灯亮起 - 注意'"/>
        </onentry>
        <transition event="next" target="red"/>
    </state>

</scxml>
```

CMake 配置。这里需要把 .scxml 文件加入编译流程。Qt 的 scxmlc 编译器会在编译期把 .scxml 转换成 C++ 代码，我们用 qt6_add_statecharts 宏来触发这个过程。生成的头文件可以直接 include 使用：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Scxml Widgets)

qt6_add_statecharts(STATECHARTS
    trafficlight.scxml)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    ${STATECHARTS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Scxml Qt6::Widgets)
```

C++ 主程序。QScxmlStateMachine::fromFile 加载状态图，连接状态变化信号更新 UI，按钮点击调用 submitEvent 触发转换。完整代码见 `examples/beginner/05-other-modules/13-qtscxml-beginner/`，下面是关键部分：

加载状态机并连接信号：

```cpp
// 从资源文件加载 SCXML 状态图
machine_ = QScxmlStateMachine::fromFile(
    QStringLiteral(":/trafficlight.scxml"));

if (!machine_) {
    qCritical() << "SCXML 文件加载失败";
    return;
}

// 监控状态变化
connect(machine_, &QScxmlStateMachine::entered,
        this, [this](const QSet<QString> &states) {
    if (states.contains("red")) {
        light_widget_->setStyleSheet(
            "background-color: red; border-radius: 75px;");
        status_label_->setText("红灯 - 停车");
    } else if (states.contains("green")) {
        light_widget_->setStyleSheet(
            "background-color: green; border-radius: 75px;");
        status_label_->setText("绿灯 - 通行");
    } else if (states.contains("yellow")) {
        light_widget_->setStyleSheet(
            "background-color: yellow; border-radius: 75px;");
        status_label_->setText("黄灯 - 注意");
    }
});

// 启动状态机
machine_->start();
```

按钮触发事件提交：

```cpp
// 点击"下一步"按钮提交 next 事件
connect(next_btn_, &QPushButton::clicked, this, [this]() {
    machine_->submitEvent(QStringLiteral("next"));
});
```

运行程序后你会看到一个圆形的灯和一个"下一步"按钮。点击按钮，灯色按 红 -> 绿 -> 黄 -> 红 的顺序循环切换。状态切换的逻辑完全在 .scxml 文件中定义，C++ 代码只负责加载状态图、监听状态变化、更新 UI 显示——状态机的结构描述和业务逻辑彻底分离。

你会发现，对比上一篇用纯 C++ 手写的状态机代码，SCXML 版本的状态结构定义要简洁得多。三个状态、三条转换，十几行 XML 就搞定了，而且一眼就能看出整个状态图的拓扑结构。如果以后需要新增状态或修改转换路径，只需要编辑 .scxml 文件，C++ 代码不用动。

## 5. 练习项目

练习项目：带自动计时的交通灯。

我们要在基础的交通灯状态图上增加自动切换功能。红灯持续 30 秒后自动切换到绿灯，绿灯持续 25 秒后切换到黄灯，黄灯持续 5 秒后切换到红灯。同时保留手动切换按钮作为调试/强制切换的入口。

完成标准是这样的：使用 `<onentry>` 中 `<send>` 标签实现延迟事件发送，`delay` 属性指定延迟时间（格式如 "30s"、"25s"、"5s"），`event` 属性指定延迟后发送的事件名；状态图中每个状态都有两条转换——一条由 "next" 事件触发（手动），一条由自动定时事件触发；C++ 界面增加一个显示剩余时间的倒计时标签；使用 SCXML 的 `<datamodel>` 定义一个 remaining_time 变量，配合定时器在 C++ 侧更新倒计时显示。

几个实现提示：`<send>` 标签的语法是 `<send event="auto_next" delay="30s"/>`，放在 `<onentry>` 里面会在状态进入时启动定时器；自动转换和手动转换可以用不同的目标状态来区分，也可以都指向同一个目标（因为转换结果相同）；倒计时可以用 C++ 侧的 QTimer 配合 `<datamodel>` 变量来实现，不需要在 SCXML 内部做复杂的定时逻辑。如果 `<send>` 标签在你的 Qt 版本中有兼容性问题，退一步在 C++ 侧用 QTimer 延迟调用 submitEvent 也完全可以达到同样的效果。

## 6. 官方文档参考

[Qt 文档 · Qt SCXML 模块](https://doc.qt.io/qt-6/qtscxml-index.html) -- SCXML 模块总览

[Qt 文档 · QScxmlStateMachine](https://doc.qt.io/qt-6/qscxmlstatemachine.html) -- 状态机引擎类

[Qt 文档 · SCXML 编译器](https://doc.qt.io/qt-6/qtscxml-compiler.html) -- scxmlc 代码生成工具

[W3C SCXML 标准](https://www.w3.org/TR/scxml/) -- SCXML 规范全文

[Qt 文档 · Qt StateMachine](https://doc.qt.io/qt-6/qtstatemachine-index.html) -- 对比参考

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。Qt SCXML 的核心价值在于把状态机的结构描述从 C++ 代码中剥离出来，用标准化的 XML 格式独立管理。.scxml 文件描述状态和转换，QScxmlStateMachine 加载运行，submitEvent 触发转换，和 QML 的集成通过 isActive/submitEvent 完成——这套组合适合需要可视化编辑状态图、跨前端共享状态逻辑的项目。对于简单的状态管理场景，上一篇学的 Qt StateMachine 更轻量直接。建议先把交通灯这个例子跑通，理解 SCXML 文件和 C++ 代码的协作方式，然后再尝试带数据模型的高级用法。
