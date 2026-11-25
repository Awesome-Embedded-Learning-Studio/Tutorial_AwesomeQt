---
title: "6.7 QML 异步进阶：WorkerScript 线程模型"
description: "QML 的 JavaScript 执行环境是单线程的——所有绑定求值、信号处理、动画回调都在同一个 GUI 线程上。如果一段 JS 代码需要跑几秒钟才能算完，整个 UI 就冻住了。WorkerScript 提供了一个独立的后台线程来执行 JS 计算，通过 sendMessage / onMessage 传递数据。再加上 QML 内置的 XMLHttpRequest 异步 HTTP 能力和 Loader 的异步组件加载，这篇把 QML 中处理异步任务的三个关键手段全讲清楚。"
---

# 现代Qt开发教程（进阶篇）6.7——QML 异步进阶：WorkerScript 线程模型

## 1. 前言

QML 的 JavaScript 执行环境是单线程的——所有绑定求值、信号处理、动画回调都在同一个 GUI 线程上跑。如果一段 JS 代码需要花几秒钟才能算完（比如解析一个巨大的 JSON、对几千条数据做排序过滤、或者做复杂的数学计算），整个 UI 就冻住了——按钮点不动，动画停摆，用户体验直接崩盘。

Qt 给了两个层面的解决方案。一个是 WorkerScript——它提供一个完全独立的后台 JS 线程，把耗时计算移过去，主线程只负责接收结果。另一个是 XMLHttpRequest——QML 内置了一个基于 Qt 网络栈的异步 HTTP 客户端，不需要 WorkerScript 也能做非阻塞的网络请求。此外，Loader 的 `asynchronous: true` 属性让你可以在后台加载复杂 QML 组件，避免启动时被大组件阻塞。这篇把这三个异步手段一口气拆完。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写。`WorkerScript` 需要 `import QtQml.WorkerScript`（Qt 6 中模块路径可能有变化，部分版本中是 `import QtQml` 直接可用）。`XMLHttpRequest` 是 QML JavaScript 环境内置的，不需要额外 import。`Loader` 在 `import QtQuick` 中。CMake 中需要 `find_package(Qt6 REQUIRED COMPONENTS Qml Quick)`。

## 3. 核心概念讲解

### 3.1 WorkerScript 基本用法——后台线程执行 JS

`WorkerScript` 的核心思路很简单：你写一个独立的 `.js` 文件，WorkerScript 在后台线程中运行这个文件的代码。主线程通过 `sendMessage()` 发数据过去，后台线程的 `onMessage` 回调收到数据、处理完毕后，再通过 `sendMessage()` 把结果发回来，主线程的 `onMessage` 收到结果。

首先写一个 Worker 脚本文件（比如 `worker.js`）：

```javascript
// worker.js —— 在后台线程中运行

/// @brief 接收主线程发来的消息，执行耗时计算后返回结果。
WorkerScript.onMessage = function(message) {
    // 模拟耗时计算：对一个数组中的每个元素做复杂运算
    var result = [];
    for (var i = 0; i < message.data.length; i++) {
        result.push(heavy_computation(message.data[i]));
    }

    // 把结果发回主线程
    WorkerScript.sendMessage({
        action: "computation_done",
        result: result
    });
};

function heavy_computation(value) {
    // 这里放你的实际计算逻辑
    return Math.sqrt(value) * Math.sin(value);
}
```

QML 侧的使用：

```qml
import QtQuick
import QtQml.WorkerScript

Item {
    WorkerScript {
        id: worker
        source: "worker.js"

        /// @brief 接收后台线程返回的结果。
        onMessage: (messageObject) => {
            // messageObject 就是 worker.js 中 sendMessage 的参数
            resultText.text = "Got " + messageObject.result.length + " results";
        }
    }

    Text { id: resultText; text: "Computing..." }

    MouseArea {
        onClicked: {
            // 发送数据到后台线程
            worker.sendMessage({
                data: [1, 4, 9, 16, 25, 36, 49, 64]
            });
        }
    }
}
```

整个通信流程是：点击 MouseArea -> `sendMessage` 把数据序列化后投递到后台线程 -> 后台线程的 `WorkerScript.onMessage` 收到数据 -> 执行耗时计算 -> `WorkerScript.sendMessage` 把结果序列化后投递回主线程 -> 主线程的 `onMessage` 收到结果 -> 更新 UI。

### 3.2 内存隔离与数据序列化

这里有一个非常重要的概念需要理解：WorkerScript 和主线程之间是**完全的内存隔离**。Worker 线程有自己独立的 JavaScript 引擎实例，它不能访问主线程的任何 QML 对象、属性、信号、或者组件。

你传递的数据（`sendMessage` 的参数）必须是**可序列化的**——数字、字符串、布尔值、普通 JavaScript 对象（键值对）、JavaScript 数组。你不能传递 QML 对象引用、`QtObject` 实例、或者任何带回调函数的东西。传递的过程本质上是深拷贝——数据被序列化为某种中间格式，然后在目标线程反序列化。这意味着如果数据量很大，传递本身就有开销。

```javascript
// 可以传递的——纯数据和简单对象
worker.sendMessage({
    numbers: [1, 2, 3],
    config: { threshold: 0.5, mode: "fast" },
    label: "result"
});

// 不能传递的——QML 对象引用
// worker.sendMessage({ item: someQmlItem });  // 运行时报错
```

这种内存隔离的设计保证了线程安全——两个线程永远不会同时访问同一个对象，不需要锁、不需要互斥量。代价就是数据传递只能走序列化这条路线，你不能通过共享内存的方式让两边同时读写同一个数据结构。

现在有一道思考题给大家。如果你的 Worker 脚本需要处理一个 10MB 的 JSON 数组，每次计算完一个批次就 `sendMessage` 回传部分结果（共 100 次），和一次性传回全部结果相比，哪种方式性能更好？

答案是取决于场景。分批传回的优点是主线程可以边收边更新 UI（进度条、部分结果展示），用户体验更好。但从纯性能角度看，100 次 `sendMessage` 的序列化/反序列化总开销比 1 次传大数据要高。如果你的数据不需要渐进展示，一次性传回效率更高；如果需要渐进更新 UI，分批传回的体验优势值得那点额外开销。

### 3.3 XMLHttpRequest——QML 中的异步 HTTP 请求

QML 的 JavaScript 环境内置了 `XMLHttpRequest`（XHR），但和浏览器中的 XHR 有些不同——QML 的实现基于 Qt 网络栈（`QNetworkAccessManager`），天然支持异步操作，不会阻塞 GUI 线程。

```javascript
/// @brief 发送异步 GET 请求并处理响应。
function fetch_data(url) {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200) {
                console.log("Response:", xhr.responseText);
                // 解析 JSON 响应
                var data = JSON.parse(xhr.responseText);
                processData(data);
            } else {
                console.log("Error:", xhr.status, xhr.statusText);
            }
        }
    };
    xhr.open("GET", url, true);  // 第三个参数 true = 异步
    xhr.send();
}
```

QML 中的 XHR 几乎总是用异步模式（第三个参数 `true`）。同步模式虽然语法上支持，但它会阻塞 GUI 线程等待响应完成——在网络不好的情况下等于 UI 冻结，和直接在主线程做耗时计算没区别。

XHR 可以在 WorkerScript 中使用——如果你需要在后台线程中做网络请求（比如下载一个大文件并解析），Worker 内部的 XHR 同样是异步的，不会阻塞 Worker 线程的消息处理。不过需要注意，Worker 内部的 XHR 回调也在 Worker 线程执行，不会影响主线程。

### 3.4 Loader 异步组件加载

`Loader` 是 QML 中动态加载组件的标准方式。默认情况下 Loader 是同步加载的——设置 `source` 或 `sourceComponent` 时，QML 引擎会立即解析并创建组件实例，如果这个组件很复杂（包含大量子项、图片资源、嵌套 Loader），加载过程可能需要几十甚至几百毫秒，期间 UI 无响应。

设置 `asynchronous: true` 后，Loader 会在后台线程解析 QML 文件，主线程继续响应用户操作。加载完成后通过 `onLoaded` 信号通知：

```qml
Loader {
    id: heavyLoader
    asynchronous: true  // 后台加载，不阻塞 UI
    source: "HeavyComponent.qml"
    visible: status === Loader.Ready

    onLoaded: {
        console.log("Heavy component loaded!");
        // 加载完成后可以安全访问 item 属性
        if (item) {
            item.configure(appSettings);
        }
    }

    // 加载中显示进度提示
    BusyIndicator {
        running: heavyLoader.status === Loader.Loading
        anchors.centerIn: parent
    }
}
```

`asynchronous: true` 特别适合应用启动时按需加载的功能模块——比如设置面板、数据报表页面、复杂的对话框。这些页面不需要在启动时就存在，等到用户第一次点击对应的入口时再异步加载，既减少了启动时间，又避免了加载过程中的 UI 卡顿。

异步 Loader 在加载完成之前 `item` 属性是 `null`。如果你在其他地方引用了 `heavyLoader.item.someProperty`，在加载完成之前会报 `Cannot read property of null` 错误。解决方案是所有对 `item` 的访问都加上 `if (item)` 守卫，或者通过 `onLoaded` 信号触发后续操作。

## 4. 踩坑预防

第一个坑是 WorkerScript 中不能访问任何 QML 对象。这一点我们前面反复提到了，但值得再强调一遍——在 `worker.js` 中你不能调用 `Qt.createComponent()`、不能访问 `Qt.application`、不能使用 `console.log` 之外的任何 QML 全局对象。如果你在 Worker 中写了 `Qt.createComponent("MyItem.qml")`，运行时会直接报 "Qt is not defined"。Worker 脚本是一个纯粹的 JavaScript 环境，唯一的对外通信渠道就是 `WorkerScript.onMessage` 和 `WorkerScript.sendMessage`。所有需要操作 QML 的逻辑必须放在主线程的 `onMessage` 回调中。

第二个坑是 `Loader` 的 `asynchronous: true` 在 Qt 6 的某些版本（6.2.x 早期）中存在一个 bug——如果异步加载的组件本身又包含了同步 Loader，嵌套的同步 Loader 可能在父 Loader 还没完全就绪时就开始创建子组件，导致组件树状态不一致。如果你遇到异步加载的组件内部出现诡妙的绑定错误或者 "Component is not ready" 警告，尝试把嵌套的 Loader 也设为 `asynchronous: true`，或者升级到 Qt 6.3+ 这个问题已经被修复了。

第三个坑是 `XMLHttpRequest` 的跨域限制。QML 中的 XHR 默认没有浏览器中的 CORS 限制——它可以直接请求任何 URL。这在方便的同时也意味着安全策略完全依赖你自己实现。如果你的应用加载了不可信的 QML/JS 代码（比如插件系统、用户脚本），那些代码可以通过 XHR 访问内网资源。如果你需要限制网络请求的范围，应该在 C++ 侧通过自定义 `QNetworkAccessManager` 来控制。

## 5. 练习项目

练习项目：异步图片处理器。界面上有一个「选择目录」按钮（用 `QFileDialog` 在 C++ 侧实现，通过 `Q_INVOKABLE` 暴露给 QML），选择目录后 WorkerScript 在后台线程遍历目录中的所有图片文件名，计算每张图片的 MD5 哈希值（模拟耗时操作），每计算完一张就通过 `sendMessage` 回传进度。QML 侧用一个进度条实时显示计算进度，并在 ListView 中逐步添加已处理的文件名和哈希值。

同时用 Loader 异步加载一个「结果面板」组件——这个组件包含一个图表（用 Canvas 画一个简单的柱状图），展示已处理文件的哈希值分布。组件在至少 10 张图片处理完成后才开始异步加载。

完成标准是选择目录后 UI 不卡顿，进度条实时更新，结果面板异步加载不阻塞进度显示。提示：WorkerScript 的数据传递只支持简单类型，文件名列表用字符串数组传递；MD5 哈希可以用一个简单的字符串处理函数模拟（不需要真正的 MD5）；Loader 的 `active` 属性可以绑定到一个条件表达式来控制何时开始加载。

## 6. 官方文档参考链接

[Qt 文档 · WorkerScript](https://doc.qt.io/qt-6/qml-qtqml-workerscript-workerscript.html) -- WorkerScript 线程通信类型

[Qt 文档 · Loader](https://doc.qt.io/qt-6/qml-qtquick-loader.html) -- 动态组件加载器

[Qt 文档 · Integrating QML and JavaScript](https://doc.qt.io/qt-6/qtqml-javascript-integratexmlhttprequest.html) -- QML 中的 XMLHttpRequest 用法

[Qt 文档 · Threaded ListModel Example](https://doc.qt.io/qt-6/qtqml-threading-threadedlistmodel-example.html) -- WorkerScript 与 ListModel 的官方示例

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。WorkerScript 把耗时 JS 计算推到后台线程，XMLHttpRequest 提供非阻塞的网络请求能力，Loader 的 `asynchronous: true` 让大组件按需加载不卡启动——这三样掌握了，QML 中的异步编程就没有盲区了。核心原则是：任何可能超过 16ms 的操作都不要在 GUI 线程做，用这三个工具中的一个把它移走，主线程永远留给 UI 渲染和用户交互。
