---
title: 信号发射入口——emit 与 activate
description: emit 是空宏，发射真身是 moc 插进信号函数体的 QMetaObject::activate；activate 把局部信号索引经 signalOffset 转全局，三个公开重载全部收敛到内部 doActivate 模板。
---

# 信号发射入口——emit 与 activate

> 本索引收录 Qt 6.9.1 源码中信号发射入口相关的已验证证据。对应专家篇《02 信号槽底层——activate 调用链》§3.1-§3.2。

## emit 空宏

源码文件：`qtbase/src/corelib/kernel/qtmetamacros.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| emit/Q_EMIT 是空宏，emit mySignal() 预处理后只是普通成员函数调用 | :49-52 | `# define Q_EMIT / # define emit` | emit 纯语法糖，编译期运行期都不认识它；信号函数体由 moc 生成。 |

## moc 在信号函数体插 activate

源码文件：`qtbase/src/tools/moc/generator.cpp`（moc 生成器模板，行号漂移快）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| moc 在无参信号函数体里插 QMetaObject::activate(this,&staticMetaObject,index,nullptr) | :1308-1311 | `fprintf(out, "QMetaObject::activate(%s, &staticMetaObject, %d, nullptr);\n", ...)` | 信号发射的本质就是调 activate；有参信号会先准备 argv 再调。 |

## activate 索引转换 + 委托 doActivate

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| activate 把 local_signal_index 经 signalOffset 转全局索引，按 signal_spy 回调委托 doActivate | :4198-4207 | `int signal_index = local_signal_index + QMetaObjectPrivate::signalOffset(m); if (Q_UNLIKELY(qt_signal_spy_callback_set...)) doActivate<true>(...) else doActivate<false>(...)` | 局部索引要加继承偏移变全局才能在连接表定位；doActivate 模板参数控制是否回调调试 spy。 |
| 三个 activate 重载（4198/4212/4226）最终都收敛到 doActivate | :4198-4226 | `doActivate<false>(sender, signal_index, argv);` | doActivate 才是信号发射真正核心。 |
