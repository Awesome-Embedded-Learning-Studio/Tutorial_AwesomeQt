---
title: 信号索引换算——signalOffset 与 originalClone
description: signalOffset 沿 superdata 链逐层累加 signalCount、methodOffset 同构、索引换算是连接期预存加发射期相加（没有换算函数，direct 慢路径 4164 相加、queued 塞进事件由 placeMetaCall 在接收线程相加）、originalClone 把 moc 默认参数克隆方法回溯到原始声明（字符串版 connect 在 3012-3013 调用）。
---

# 信号索引换算——signalOffset 与 originalClone

> 本索引收录 Qt 6.9.1 源码中信号索引换算与克隆方法回溯相关的已验证证据。对应专家篇《18 信号槽深度拆解》正文辅助件一节。

## 索引偏移

源码文件：`qtbase/src/corelib/kernel/qmetaobject_p.h` / `qmetaobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| signalOffset 沿 superdata 链爬，逐层累加 signalCount | qmetaobject_p.h:155-162 | `for (m = m->d.superdata; m; m = m->d.superdata) offset += ... signalCount;` | 从直接父类起算，返回本类首个信号的全局索引。 |
| methodOffset 同构（累加 methodCount） | qmetaobject.cpp:479-487 | 同构循环 | 两者的求和使用点在 activate 重载（qobject.cpp:4201、4214）。 |
| 换算无函数：连接期预存、发射期相加 | qobject.cpp:3609 / :3646 / :4148 / :4164 | 字符串版 connect 3609 算 method_offset、3646 存；direct 快路径 4148 只传相对索引；慢路径 4164 `const int method = c->method_relative + c->method_offset;` | convertSignalIndexToMethodIndex 全库不存在。queued 路径把两个数塞进 QMetaCallEvent（3991），由 placeMetaCall 在接收线程相加（626-627）。 |

## 克隆方法回溯

源码文件：`qtbase/src/corelib/kernel/qmetaobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| originalClone 沿索引向下回溯 MethodCloned 标记 | :4498-4506 | `while (QMetaMethod::fromRelativeMethodIndex(mobj, local_method_index).data.flags() & MethodCloned) { --local_method_index; }` | moc 为默认参数生成克隆方法并打标；函数注释举的例子是 destroyed() 回到 destroyed(QObject*)。原版（带全参）索引更小在前，丢参克隆在后。 |
| 字符串版 connect 直接调 originalClone 再叠 signalOffset | qobject.cpp:3012-3013 | 原文见函数本体 | 整数索引版（QMetaObject::connect 3586）走 methodIndexToSignalIndex（3553，内含 3565 调用），两条路径不同源。声明在 qmetaobject_p.h:131。 |
