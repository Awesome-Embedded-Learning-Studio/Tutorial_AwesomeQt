---
title: "卡住怎么办"
description: "按症状查：信号没触发、connect 连不上、跨线程崩、lambda 连接野指针、detach 忘了崩——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `model/12-design-patterns/observer-pattern/`，对照着看。

## 点按钮后面板没刷新（信号没触发 / connect 没连上）

- `QtSubject` 类**有没有 `Q_OBJECT`**？没有的话 moc 不生成信号元数据，`connect` 编译期找不到信号。→ `include/observer-pattern.h:68`
- `setValue` 里**有没有真的 `emit valueChanged(...)`**？只改了成员变量没 emit = 没人通知。→ `src/observer-pattern.cpp:50`
- 是不是用了**老式 `SIGNAL()/SLOT()` 字符串宏**且签名拼错了？这种错运行期才报「连不上」，且只在 debug 输出里有。换 Qt6 函数指针语法。→ `demo/observer-pattern_window.cpp:145`
- CMake **有没有开 `AUTOMOC`**？→ `CMakeLists.txt`
- 进阶排查：[信号与槽（入门）](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

## 纯 C++ 路径偶发崩溃（segfault）

- `ClassicSubject::notify` 遍历的 `Observer*` **是不是有已析构但没 detach 的**？裸指针不跟踪生命周期，悬空即崩。→ `src/observer-pattern.cpp:35`
- `attach` 之前**有没有判空**？挂了空指针，notify 时空跳虽然不崩但不规范。→ `src/observer-pattern.cpp:17`
- 进阶排查：这是纯 C++ observer 的固有缺陷，Qt 信号槽用对象树免掉了它（见成品导览踩坑①）

## disconnect 按信号+lambda 断不掉

- lambda **没有 `operator==`**，`disconnect(sender, &Sig, receiver, functor)` 找不回原 functor。→ 注释在 `demo/observer-pattern_window.cpp:203`
- 正路：**存 `connect` 返回的 `QMetaObject::Connection`**，`disconnect(conn)` 一行断。→ `demo/observer-pattern_window.cpp:209`

## lambda 连接里捕获的指针偶尔野（崩溃）

- connect 是不是**没给 context**（写成 3 参 `connect(sender, sig, lambda)`）？这种连接不跟随任何对象，捕获的 `this`/裸指针被销毁后仍被调 → **segfault**。
- 正路：connect **第 3 参给 context 对象**，context 销毁时连接自动失效。→ `demo/observer-pattern_window.cpp:224`
- 进阶排查：[信号与槽（进阶）](../../../../../advanced/01-qtbase/02-signal-slot-advanced.md)

## 跨线程信号槽槽函数跑在错线程 / 崩

- 是不是把连接类型**写死成 `DirectConnection` 又跨了线程**？这样槽跑在 sender 线程，操作 GUI 控件必崩。
- 跨线程用 `Qt::QueuedConnection`，或干脆留默认 `AutoConnection`（同线程直连、跨线程自动队列）。→ `demo/observer-pattern_window.cpp:183`
- 进阶排查：[多线程（入门）](../../../../../beginner/01-qtbase/09-multithreading-beginner.md)、[QThread（进阶）](../../../../../advanced/01-qtbase/09-qthread-advanced.md)

## 切了连接类型但面板行为没变

- `connect(... , type)` 的 **5 参重载**是不是少传了 context？lambda 重载和 `ConnectionType` 参数匹配需要 context（第 3 参）。→ `demo/observer-pattern_window.cpp:186`
- 是不是**断开没成功**就重连了？重连前先 `disconnect(history_conn_)`，否则同一个信号挂两份槽，两个都跑。→ `demo/observer-pattern_window.cpp:175`

## moc 报错（信号 / Q_ENUM 不认识）

- 头文件**有没有 `Q_OBJECT`**？→ `include/observer-pattern.h:68`
- CMake **有没有开 AUTOMOC**？→ `CMakeLists.txt`
- `Q_ENUM` 的枚举**是不是在 `Q_OBJECT` 类里**、Q_ENUM 紧跟其后？→ `include/observer-pattern.h:72-76`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
