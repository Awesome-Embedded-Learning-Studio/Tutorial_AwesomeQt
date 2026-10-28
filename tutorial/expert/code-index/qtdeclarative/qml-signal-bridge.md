---
title: QML 信号桥——notifyList 快通道与 V4 调用链
description: onXxx handler 不走 QObject::connect 而是经 QQmlNotifierEndpoint 挂进 QQmlData::notifyList、doActivate 的 signalEmitted 钩子直通 emitNotify 且无普通连接时整段跳过、跨线程由 QQmlThreadNotifierProxyObject 搬运、QQmlPropertyCache 不再继承 QMetaObject（final 引用计数类加惰性 createMetaObject）、QQmlObjectOrGadget 的 QBiPointer 双态包装与三分支 metacall、V4 侧 virtualCall 到 CallMethod 打包 argv 的当代链路。
---

# QML 信号桥——notifyList 快通道与 V4 调用链

> 本索引收录 Qt 6.9.1 源码中 QML 引擎与 Qt 元对象系统之桥相关的已验证证据。对应专家篇《18 信号槽深度拆解》正文 QML 桥一节。发射侧钩子在 qtbase 的证据一并收录于此。

## onXxx 的真实连接路径

源码文件：`qtdeclarative/src/qml/qml/qqmlboundsignal.cpp` / `qqmlnotifier.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QQmlBoundSignal 构造本身就是 QQmlNotifierEndpoint，经 endpoint connect 登记进 notifyList | qqmlboundsignal.cpp:213-229 | `signal = QQmlPropertyCache::originalClone(target, signal); QQmlNotifierEndpoint::connect(target, signal, engine);`（227-228） | 全文件无 Qt::ConnectionType；QQmlNotifierEndpoint::connect 内部 ddata->addNotify(sourceSignal, this)（qqmlnotifier.cpp:105-106），与 QObjectPrivate 连接列表零交集。 |
| endpoint 侵入式链表与回调表 | qqmlnotifier_p.h:47-48 / qqmlnotifier.cpp:18-24 | next/prev 成员；QQmlNotifier_callbacks 裸函数指针表 | 绑定走 QQmlJavaScriptExpressionGuard 槽位。 |
| emitNotify 整条入栈再逆序调回调 | qqmlnotifier.cpp:46-75 | `QVarLengthArray<NotifyListTraversalData> stack; while (endpoint) { stack.append(...); endpoint = endpoint->next; }`（46-53 起步） | 同线程发射的同步直调入口。 |
| endpoint 编址用 signalIndex 范围而非 methodIndex | qqmldata_p.h:352-355 | 注释原文 | notifyList 用 connectionMask 加按索引编址的稀疏数组（126-136 结构，写入 qqmlengine.cpp:1298-1313 一带）。 |

## 发射侧钩子与跨线程搬运

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qtdeclarative/src/qml/qml/qqmlengine.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| doActivate 先经 QAbstractDeclarativeData::signalEmitted 钩子回 QML | qobject.cpp:4033-4038 | signalEmitted 钩子调用段 | isDeclarativeSignalConnected 是独立前置检查。 |
| 信号只有 QML endpoint、无普通连接时 doActivate 处理完钩子直接 return | qobject.cpp:4046-4052 | `if (!connections->maybeSignalConnected(signal_index)) ... return;` 一带 | maybeSignalConnected（448 行起）只查普通连接表；普通连接列表整段不遍历，这就是快通道的收益点。 |
| 同线程：signalEmitted 直接 ddata->notify 调 emitNotify | qqmlengine.cpp:343-346 | `ddata->notify(index)` 一带 | QQmlBoundSignal_callback（qqmlboundsignal.cpp:294）跑 JS handler。 |
| 跨线程：打包 QMetaCallEvent 投给 QQmlThreadNotifierProxyObject 搬回对象线程 | qqmlengine.cpp:307-341 | postEvent 到代理对象，代理 qt_metacall 里再调 emitNotify（272-286） | 机制上与 QueuedConnection 的差别：无 QMetaObject::Connection 句柄、无 sender 恢复。 |

## QQmlPropertyCache 与 QQmlObjectOrGadget

源码文件：`qtdeclarative/src/qml/qml/qqmlpropertycache_p.h` / `qqmlobjectorgadget_p.h` / `qqmlobjectorgadget.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QQmlPropertyCache 不再继承 QMetaObject（Qt5 旧形态已失效） | qqmlpropertycache_p.h:119-121 | `class Q_QML_EXPORT QQmlPropertyCache final : public QQmlRefCounted<QQmlPropertyCache>` | 全文件搜不到 public QMetaObject；缓存本体是 QVector 索引表加名字表（253-255），需要时 createMetaObject()（声明 165，实现 qqmlpropertycache.cpp:322-331）经 QMetaObjectBuilder 惰性生成，基类链由 setSuperClass 递归接上（327）。 |
| QQmlObjectOrGadget 用 QBiPointer 双态包装 QObject 与 gadget | qqmlobjectorgadget_p.h:23-44 | `QQmlObjectOrGadget(QObject *obj) / QQmlObjectOrGadget(const QMetaObject *, void *gadget)`（26-33），`QBiPointer<QObject, void> ptr`（44） | 元数据在基类 QQmlMetaObject，只存 const QMetaObject* _m（qqmlmetaobject_p.h:170）；QQmlPropertyCache 构造时即 createMetaObject 物化（185-189）。 |
| metacall 三分支：QObject 走 QMetaObject::metacall，gadget 先解析再 static_metacall | qqmlobjectorgadget.cpp:8-21 | `else if (ptr.isT1()) { QMetaObject::metacall(...); } else { resolveGadgetMethodOrPropertyIndex(...); metaObject->d.static_metacall(...); }` | 空指针分支（10-12）当前唯一使用者是 JS 构造路（qv4qmetaobjectwrapper.cpp:71）。qtbase 侧 QMetaObject::metacall 先查动态元对象（qmetaobject.cpp:343-349）。 |

## V4 调用链（函数名已换代）

源码文件：`qtdeclarative/src/qml/jsruntime/qv4qobjectwrapper.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 当代链路：virtualCall 到 callInternal 到 resolveOverloaded 到 callPrecise 到 CallMethod | :2956-2961 / :2970 / :2099 / :2012 / :1667 | 函数定义位置 | callMethodWithInstance/callOverloaded 全库不存在；callInternal 在 3005-3026 把对象重打包成 QQmlObjectOrGadget；另有带类型通道 callInternalWithMetaTypes（3088）。 |
| CallMethod 把 JS 实参按 QMetaType 转进 CallArgument 再收进 void* 数组 | :1673-1677 / :1700-1706 | `argData[ii] = args[ii].dataPtr(); object.metacall(callType, index, argData.data()); return args[0].toValue(engine);` | args[0] 留给返回值；这一行 metacall 就是 V4 与 qtbase 元对象系统的合龙点。 |
