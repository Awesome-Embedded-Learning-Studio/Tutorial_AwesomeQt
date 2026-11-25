---
title: "5.13 SCXML 进阶：数据模型与延迟事件"
description: "入门篇我们把 QScxmlStateMachine 的基本 SCXML 加载跑通了——加载一个 .scxml 文件、启动状态机、触发事件。但 SCXML 规范远不止简单的状态转移——它内置了数据模型（`<data>` 元素）、条件表达式、延迟事件（`<send delay>`）和外部通信能力。"
---

# 现代Qt开发教程（进阶篇）5.13——SCXML 进阶：数据模型与延迟事件

## 1. 前言

入门篇我们把 QScxmlStateMachine 的基本 SCXML 加载跑通了——加载一个 .scxml 文件、启动状态机、触发事件。但 SCXML 规范远不止简单的状态转移——它内置了数据模型（`<data>` 元素）、条件表达式、延迟事件（`<send delay>`）和外部通信能力。

SCXML（State Chart XML）是 W3C 标准的状态机描述语言。和 Qt 原生 QStateMachine 的 C++ API 不同，SCXML 把状态机逻辑完全放在 XML 文件里，C++ 代码只负责加载和触发事件。这意味着状态机逻辑可以由非程序员（比如交互设计师）编写和修改，不需要重新编译 C++ 代码。

这篇我们把 SCXML 的数据模型操作、条件转移、延迟事件发送这三个高级特性拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::Scxml 模块，CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Scxml)` 引入。Qt 提供了 `qt6_add_scxml()` CMake 宏来编译 SCXML 文件生成 C++ 头文件。

## 3. 核心概念讲解

### 3.1 数据模型——`<datamodel>` 与 `<data>`

SCXML 的 `<datamodel>` 元素定义状态机的内部数据。这些数据可以在条件表达式和动作中引用。

```xml
<scxml xmlns="http://www.w3.org/2005/07/scxml" version="1.0"
       datamodel="ecmascript">
  <datamodel>
    <data id="retryCount" expr="0"/>
    <data id="maxRetries" expr="3"/>
  </datamodel>

  <state id="idle">
    <transition event="start" target="connecting"/>
  </state>

  <state id="connecting">
    <onentry>
      <assign location="retryCount" expr="retryCount + 1"/>
    </onentry>
    <transition event="connected" target="connected"/>
    <transition event="error" cond="retryCount &lt; maxRetries"
                target="connecting"/>
    <transition event="error" cond="retryCount >= maxRetries"
                target="failed"/>
  </state>

  <state id="connected">
    <transition event="disconnect" target="idle">
      <assign location="retryCount" expr="0"/>
    </transition>
  </state>

  <state id="failed" final="true"/>
</scxml>
```

C++ 侧可以读取和修改数据模型的值：

```cpp
auto *sm = QScxmlStateMachine::fromFile("connection.scxml");
sm->start();

// 读取数据模型
int retries = sm->dataModel()->scxmlProperty("retryCount").toInt();

// 设置数据模型
sm->dataModel()->setScxmlProperty("maxRetries", QVariant(5));

// 触发事件
sm->submitEvent("start");
```

### 3.2 延迟事件——`<send delay>`

SCXML 的 `<send>` 元素可以延迟发送事件，用于实现定时器和超时。

```xml
<state id="waiting">
  <onentry>
    <send event="timeout" delay="5s"/>
  </onentry>
  <transition event="response" target="processing"/>
  <transition event="timeout" target="failed"/>
  <onexit>
    <cancel sendid="timeout"/>
  </onexit>
</state>
```

进入 `waiting` 状态时发送一个延迟 5 秒的 `timeout` 事件。如果在 5 秒内收到 `response` 事件就转到 `processing`；超时则转到 `failed`。`<cancel>` 在退出 `waiting` 时取消未触发的定时器——这很重要，否则即使已经转移走了，timeout 事件还是会触发。

### 3.3 C++ 与 SCXML 的双向交互

SCXML 可以通过 `<send>` 向外部发送事件，C++ 侧通过 `QScxmlStateMachine::connectToEvent()` 接收：

```cpp
sm->connectToEvent("notify", [](const QScxmlEvent &event) {
    QString message = event.data().toString();
    qDebug() << "SCXML notification:" << message;
});
```

## 4. 踩坑预防

SCXML 的 `datamodel="ecmascript"` 要求 ECMAScript 表达式支持。Qt 使用 QML 引擎来执行这些表达式，这意味着你的 Qt 安装必须包含 QML 模块。如果用 `datamodel="cplusplus"`（Qt 扩展），表达式语法不同但不需要 QML。

## 5. 练习项目

练习项目：网络重连状态机。用 SCXML 实现指数退避重连逻辑。数据模型维护 retryCount 和 maxRetries。延迟事件实现超时检测。C++ 侧监听状态变化并更新 UI。

## 6. 官方文档参考链接

[Qt 文档 · QScxmlStateMachine](https://doc.qt.io/qt-6/qscxmlstatemachine.html) -- SCXML 状态机核心 API

[Qt 文档 · Qt SCXML](https://doc.qt.io/qt-6/qtscxml-index.html) -- SCXML 模块总览

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。SCXML 的数据模型、条件转移和延迟事件——让你能用 XML 描述复杂的状态机逻辑，C++ 只负责桥接。
