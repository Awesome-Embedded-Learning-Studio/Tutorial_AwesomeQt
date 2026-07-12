---
title: QTranslator 与 QM 文件格式源码索引
description: tr() 由 Q_OBJECT 的 QT_TR_FUNCTIONS 植入、tr 找不到翻译返回原文非空串、translate 遍历 translators 列表 prepend 故后装优先、installTranslator 不去重、QM 文件 16 字节随机二进制 magic（非可读串）、QM 无版本号字段 Qt5/Qt6 兼容、四子节 Contexts/Hashes/Messages/NumerusRules、context 查找哈希+桶线性、消息查找二分搜索预排序哈希数组（非哈希表）、ELF hash、getMessage 变长 TLV 精确匹配、translation 是 UTF-16 大端（非 UTF-8）sourceText 是 UTF-8、复数 NumerusRules 字节码虚拟机、disambiguation=comment 新旧名并存、Dependencies 链式查找、QTranslator 析构自摘除（QCoreApplication 不拥有）。
---

# QTranslator 与 QM 文件格式源码索引

> 本索引收录 Qt 6.9.1 源码中 QTranslator + tr() + QM 文件格式的已验证证据。tr 经 staticMetaObject（moc 生成）拿 context，见 [QMetaObject 静态元数据](./qmetaobject-static-metadata.md)。

## tr() 宏链

源码文件：`qtbase/src/corelib/kernel/qtmetamacros.h` / `qmetaobject.cpp` / `qobject.h` / `qcoreapplication.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| tr 由 QT_TR_FUNCTIONS 植入 | qtmetamacros.h:84-92 | `static inline QString tr(...) { return staticMetaObject.tr(s, c, n); }` | inline static，转发 staticMetaObject.tr；每个 Q_OBJECT 类都有自己的。 |
| 无 Q_OBJECT 没 tr | qtmetamacros.h:133-146 | Q_OBJECT 宏内含 `QT_TR_FUNCTIONS` | 无 Q_OBJECT 的类 staticMetaObject 不存在，调 tr 编译失败。 |
| tr→QMetaObject::tr 填 className | qmetaobject.cpp:416-419 | `return QCoreApplication::translate(className(), s, c, n);` | context 是 moc 编译期烧进 staticMetaObject 字符串表的类名。 |
| 非 QObject 类用 Q_DECLARE_TR_FUNCTIONS | qcoreapplication.h:257-261 | `translate(#context, sourceText, ...)` | #context 字符串化作 context，绕过 staticMetaObject。 |
| QT_NO_TRANSLATION 时 tr 回退 fromUtf8 | qobject.h:119-122 | `return QString::fromUtf8(sourceText);` | tr 仍存在不报错，代码不必 #ifdef 包裹。 |

## translate 查找链

源码文件：`qtbase/src/corelib/kernel/qcoreapplication.cpp` / `qcoreapplication_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【关键纠偏】找不到返原文非空串 | qcoreapplication.cpp:2283-2311 | `if (result.isNull()) result = QString::fromUtf8(sourceText);` | 遍历 translators 全 miss 返 sourceText 的 UTF-8 解码；只有 sourceText==nullptr 才返空。 |
| 后装的 translator 优先 | qcoreapplication.cpp:2294-2303 | constBegin→constEnd 正向遍历 + installTranslator prepend | 列表头部是最新装的，故最新装的先被查询。 |
| 【纠偏】installTranslator 不去重 | qcoreapplication.cpp:2165-2176 | `d->translators.prepend(translationFile);` 无 contains | 同 translator install 两次=列表两条目。 |
| removeTranslator removeAll + 条件发事件 | qcoreapplication.cpp:2201-2220 | `if (d->translators.removeAll(translationFile)) { ...sendEvent LanguageChange... }` | removeAll 删全部匹配项；未找到不发事件。 |
| install 后 sendEvent LanguageChange | qcoreapplication.cpp:2183-2186 | `QEvent ev(QEvent::LanguageChange); sendEvent(self, &ev);` | 同步派发，触发 UI retranslateUi。 |
| translators 受 QReadWriteLock 保护 | qcoreapplication_p.h:158-162 | `QTranslatorList translators; QReadWriteLock translateMutex;` | 读 translate 用 ReadLocker，写 install/remove 用 WriteLocker。 |

## QM 文件格式

源码文件：`qtbase/src/corelib/kernel/qtranslator.cpp` / `qtranslator_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【关键纠偏】16 字节随机二进制 magic | qtranslator.cpp:51-62 | `magic[16]={0x3c,0xb8,0x64,0x18,0xca,0xef,0x9c,0x95,0xcd,0x21,0x1c,0xbf,0x60,0xa1,0xbd,0xdd}` | 源自 mcookie 命令随机生成；非可读字符串。 |
| 【关键纠偏】无版本号字段 | qtranslator.cpp:260+794-836 | `enum { Contexts=0x2f, Hashes=0x42, Messages=0x69, ... };` + TLV 循环 | magic 后立即进 1字节tag+4字节big-endian长度循环；Qt5/Qt6 兼容。 |
| 四子节 + 依赖 + 语言 | qtranslator.cpp:256-291 | messageArray/offsetArray/contextArray/numerusRulesArray | Hashes 内存里对应 offsetArray（命名错位历史遗留）。 |
| load 校验 magic | qtranslator.cpp:539-544+773-774 | `memcmp(magicBuffer, magic, MagicLength)` | 长度<16 或 magic 不符立即返回 false。 |
| Unix 优先 mmap | qtranslator.cpp:548-581 | `PROT_READ \| MAP_PRIVATE` + `QT_MMAP` | 失败回退 new+readAll；翻译查询零拷贝。 |
| 资源内未压缩 QM 零拷贝 | qtranslator.cpp:511-527 | 复用 `QResource::data()` 指针 | NoCompression + magic 匹配时直接拿指针。 |

## translate 查找算法

源码文件：`qtbase/src/corelib/kernel/qtranslator.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【关键纠偏】context 哈希+桶线性 | qtranslator.cpp:957-976 | `elfHash(context) % hTableSize` 落桶 + 桶链 match | context 不存在立即返空，不浪费后续查找。 |
| 【关键纠偏】消息二分搜索非哈希表 | qtranslator.cpp:985-1023 | offsetArray 预排序哈希数组，每项 8 字节，`while(start<=end)` 二分 | hash = elfHash_continue(sourceText) 后 continue(comment) 累加（非 XOR）；命中后扫相同 hash 项。 |
| ELF hash | qtranslator.cpp:75-101 | `h=(h<<4)+*k++; if((g=h&0xf0000000)) h^=g>>24; h&=~g;` | continue 模式可拼接 sourceText+comment 进同一哈希状态机。 |
| getMessage 变长 TLV 精确匹配 | qtranslator.cpp:870-935 | Tag_Translation/SourceText/Context/Comment + match | 哈希碰撞靠精确字符串匹配排除；numerus 倒计数器选复数形式。 |
| 【关键纠偏】translation 是 UTF-16BE | qtranslator.cpp:929-934 | `QString str(tn_length/2, Qt::Uninitialized); qFromBigEndian<char16_t>(tn, str.size(), str.data());` | translation 字段 UTF-16 大端；sourceText/Language 是 UTF-8。 |

## 复数 / disambiguation / 依赖

源码文件：`qtbase/src/corelib/kernel/qtranslator.cpp` / `qtranslator_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 复数 NumerusRules 字节码虚拟机 | qtranslator.cpp:177-254, qtranslator_p.h:20-41 | numerusHelper 栈式执行 Q_MOD_10/Q_EQ/Q_NEWRULE 等 | result 累加=匹配的复数形式索引；多复数语言多条 Q_NEWRULE。 |
| disambiguation 进哈希+精确匹配 | qtranslator.cpp:986-989+917-923 | elfHash_continue(comment,h) + Tag_Comment match | 同名 sourceText 不同 disambiguation 得不同 hash；QM 空 comment 不强制匹配。 |
| comment miss 回退 comment="" 重查 | qtranslator.cpp:1024-1027 | 外层 for(;;) 两轮 | 带 disambiguation 查询也能命中无 disambiguation 译文。 |
| Dependencies 链式 subTranslators | qtranslator.cpp:1029-1035+841-849 | 主 miss 后递归调子；任一依赖加载失败全清 | 一个 QM 可声明依赖其他 QM。 |

## 杂项

源码文件：`qtbase/src/corelib/kernel/qcoreapplication.cpp` / `qtranslator.cpp` / `qttranslation.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| qtTrId 把 id 当 sourceText | qcoreapplication.cpp:2313-2317 | `translate(nullptr, id, nullptr, n)` | 基于 ID 的全局翻译，不依赖类名 context。 |
| NOOP 宏编译期退化 | qttranslation.h:19-38 | `#define QT_TR_NOOP(x) x` | 唯一作用是给 lupdate 标记，提取非 tr() 调用点的待译串。 |
| replacePercentN 替换 %n/%Ln | qcoreapplication.cpp:2222-2248 | indexOf('%') + arg(n) | translate 末尾调用；tr("%n file(s)",nullptr,count) 的底层。 |
| clear() postEvent LanguageChange 异步 | qtranslator.cpp:1042-1076 | `postEvent(...new QEvent(LanguageChange))` | 对比 install/remove 的 sendEvent 同步；hot-swap QM 后 UI 感知。 |
| 【纠偏】析构自摘除 | qtranslator.cpp:406-412 | `~QTranslator() { if (QCoreApplication::instance()) removeTranslator(this); ... }` | QCoreApplication 不拥有所有权（doc does not take ownership），但析构自摘除防悬挂。 |
| disambiguation = comment 新旧名 | qtranslator.cpp:1096-1101 | public translate 第三参 disambiguation；private do_translate 第三参 comment | 同一个东西，源码私有层保留 Qt4 旧名。 |
