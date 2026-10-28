---
title: glib 分发器——兄弟后端与默认判定链
description: QEventDispatcherGlib 与 UNIX 版平级（同继承 QAbstractEventDispatcherV2）复用同一张 QTimerInfoList、等待主体是 g_main_context_iteration、四个 GSource（事件/socket/定时器/空闲）构造期创建并 attach、glib 版无 interrupt 标志位只调 wakeUp（serialNumber 加 g_main_context_wakeup）、版本阈值 glib 2.3.1（0x020301 编译期检查）、Linux 默认 dispatcher 判定链（编译宏 QT_NO_GLIB 与运行时同名环境变量分开、官方默认构建且无环境变量覆盖时 glib 优先）。
---

# glib 分发器——兄弟后端与默认判定链

> 本索引收录 Qt 6.9.1 源码中 QEventDispatcherGlib 与默认分发器选择相关的已验证证据。对应专家篇《20 事件循环源码》正文平台后端下潜的 glib 一节与开篇判定链。

## glib 后端本体

源码文件：`qtbase/src/corelib/kernel/qeventdispatcher_glib.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 等待主体是 g_main_context_iteration，poll 被 GMainContext 调度取代 | :399-401 | `bool result = g_main_context_iteration(d->mainContext, canWait); while (!result && canWait) result = ...;` | 与 UNIX poll 版是平级兄弟（glib_p.h:27 同继承 QAbstractEventDispatcherV2），复用 QTimerInfoList（GTimerSource 内嵌，92-98）。 |
| 四个 GSource 构造期创建并 attach | :289-322（构造） | GPostEventSource / GSocketNotifierSource / GTimerSource / GIdleTimerSource | 各自认领事件、socket、定时器、空闲四摊活；GSource 是 glib 主循环的事件源基类型，靠 prepare/dispatch 回调参与调度。 |
| glib 版无 interrupt 标志位，interrupt 只调 wakeUp | :559-569 | `void QEventDispatcherGlib::interrupt() { ... wakeUp(); }` | wakeUp 是 serialNumber.ref() 加 g_main_context_wakeup；退出语义靠 GPostEventSource 的 serialNumber。 |
| 版本阈值 glib 2.3.1 非 2.30 | :571-578 | `versionSupported()` 编译期检查 0x020301 | 阈值常量十六进制拆开是 2.3.1。 |

## 默认判定链

源码文件：`qtbase/src/corelib/thread/qthread_unix.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| createEventDispatcher 的条件分支 | :319-342（关键 331-338） | 331 编译宏 QT_NO_GLIB、333 运行时同名环境变量、334 主线程或未设 QT_NO_THREADED_GLIB、335 版本检查 | 官方默认构建（configure 检测到系统 glib）且运行时未设 QT_NO_GLIB 环境变量时 glib 优先，自编译 -no-glib 或环境变量覆盖回 UNIX poll 版。断言必须带前提，禁写成对所有 Linux 发行版无条件成立。xcb 侧类似但少 QT_NO_THREADED_GLIB 一层（qxcbeventdispatcher.cpp:104，背景引用）。 |
