---
title: spy 回调——调试工具安插的耳朵
description: QSignalSpyCallbackSet 四个回调函数指针（字段 snake_case，住在私有头 qobject_p.h）、qt_register_signal_spy_callbacks storeRelease 写全局原子指针、doActivate 的调用顺序 signal_begin 每连接 slot_begin/slot_end signal_end、无人订阅短路时也成对调 begin 与 end、activate 三重载按回调集是否存在选 doActivate 模板分支。
---

# spy 回调——调试工具安插的耳朵

> 本索引收录 Qt 6.9.1 源码中信号侦听回调相关的已验证证据。对应专家篇《18 信号槽深度拆解》正文辅助件一节。

## 结构与注册

源码文件：`qtbase/src/corelib/kernel/qobject_p.h` / `qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QSignalSpyCallbackSet 四回调，字段 snake_case，在私有头非 qobjectdefs.h | qobject_p.h:48-57 | `BeginCallback signal_begin_callback, slot_begin_callback; EndCallback signal_end_callback, slot_end_callback;` | 两 Begin 带 argv、两 End 不带；注释标明 for Qt Test。 |
| 注册函数 storeRelease 写全局原子指针 | qobject.cpp:63-66 | `qt_register_signal_spy_callbacks` 写 qt_signal_spy_callback_set（61 行声明） | QSignalSpy/GammaRay 这类工具的接入点。 |

## 调用点顺序

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| signal_begin 在发射前调 | :4055-4056 | `if (callbacks_enabled && signal_spy_set->signal_begin_callback != nullptr) signal_spy_set->signal_begin_callback(sender, signal_index, argv);` | 连无人订阅被短路（4048-4051）也先成对调 begin 与 end 再返回。 |
| 每连接槽前后 slot_begin / slot_end | :4153-4154 / :4161-4162 / :4166-4167 / :4175-4176 | static_metacall 快路径与 metacall 慢路径各一对 | 槽执行被 spy 成对包裹。 |
| signal_end 在全部连接跑完且 sender 未死时调 | :4187-4191 | `if (!senderDeleted) ... signal_end_callback(...)` | 发射途中对象析构则收尾回调不发。 |
| activate 三重载按回调集选 doActivate 模板分支 | :4203-4206 / :4216-4219 | `Q_UNLIKELY(qt_signal_spy_callback_set.loadRelaxed())` 命中走 doActivate<true> | 模板声明在 4023，回调集 4040 加载一次；没装 spy 时零开销。 |
