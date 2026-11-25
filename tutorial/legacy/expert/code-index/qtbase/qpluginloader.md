---
title: QPluginLoader 与 QFactoryLoader 源码索引
description: QPluginLoader 继承 QObject（非 QLibrary 子类）持 QLibraryPrivate*、instance() 双层单例（QLibraryPrivate::inst QPointer 缓存 + 插件层 static QPointer）、Q_PLUGIN_METADATA 宏零展开由 moc 生成、Qt6 metadata 是 CBOR 编码（非 Qt5 JSON 文本）、iid 校验只在 QFactoryLoader（QPluginLoader 不校验）、CBOR 流式扫描不 dlopen 读 metadata、QFactoryLoader 三层缓存+PreventUnloadHint、版本兼容「minor 允许老不允许新」、同 key 择优「宁老勿新」、Qt5.7+ 默认 PreventUnloadHint 导致 dlclose 是 no-op、双计数器 libraryRefCount/libraryUnloadCount、静态插件 qt_static_plugin_##MANGLEDNAME。
---

# QPluginLoader 与 QFactoryLoader 源码索引

> 本索引收录 Qt 6.9.1 源码中 QPluginLoader + QFactoryLoader 的已验证证据。moc 生成插件导出代码的过程见 [MOC 编译器原理](./moc-codegen.md)。

## QPluginLoader 与 QLibrary 的关系

源码文件：`qtbase/src/corelib/plugin/qpluginloader.h` / `qpluginloader.cpp` / `qlibrary_unix.cpp` / `qlibrary_win.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 继承 QObject 非 QLibrary | qpluginloader.h:20-22 | `class Q_CORE_EXPORT QPluginLoader : public QObject` | 与 QLibrary 平级兄弟，不是父子。 |
| 持 QLibraryPrivate* 组合非封装 | qpluginloader.h:17+48-51 | 前向声明 `QLibraryPrivate` + 私有 `QLibraryPrivate *d` | 与 QLibrary 共用同一层内部实现。 |
| d 指针经 findOrCreate 获取 | qpluginloader.cpp:307-309 | `d = QLibraryPrivate::findOrCreate(fn, ...); if (!fn.isEmpty()) d->updatePluginState();` | 共享实例，多 loader 同文件复用同一 QLibraryPrivate。 |
| Unix dlopen 在 qlibrary_unix.cpp | qlibrary_unix.cpp:199 | `hnd = dlopen(QFile::encodeName(attempt), dlFlags);` | 加载链最底层：QPluginLoader→loadPlugin→load→load_sys→dlopen。 |
| Windows LoadLibrary 在 qlibrary_win.cpp | qlibrary_win.cpp:58 | `hnd = LoadLibrary(reinterpret_cast<const wchar_t*>(...));` | 源码逐字是 LoadLibrary（UNICODE 展开为 LoadLibraryW）。 |

## instance() 双层单例

源码文件：`qtbase/src/corelib/plugin/qpluginloader.cpp` / `qlibrary.cpp` / `qlibrary_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【关键纠偏】pluginInstance 缓存单例 | qlibrary.cpp:499-524, qlibrary_p.h:96 | `QPointer<QObject> inst; ... if (obj) return obj; ... obj = factory(); if (inst) obj = inst; else inst = obj;` | inst 是 QPointer（弱引用），命中返回同一指针；对象死后重走 factory。常见教材「每次新对象」错。 |
| instance() 转调 d->pluginInstance() | qpluginloader.cpp:139-144 | `if (!isLoaded() && !load()) return nullptr; return d->pluginInstance();` | QPluginLoader 层不 new，缓存逻辑在 QLibraryPrivate。 |
| 工厂函数 instanceFactory 原子缓存 | qlibrary.cpp:581-599 | `resolve("qt_plugin_instance")` → `instanceFactory.storeRelease(ptr)` | 无锁双检（注释 two threads may store same value）；符号名固定 qt_plugin_instance 无 mangling。 |
| 插件层 static QPointer 单例 | qplugin.h:212-220 | `static QPointer<QObject> _instance; if (!_instance) _instance = new IMPLEMENTATION; return _instance;` | Q_PLUGIN_INSTANCE 宏，叠加 QLibraryPrivate::inst 是双层单例。 |

## Q_PLUGIN_METADATA 宏与 CBOR 编码

源码文件：`qtbase/src/corelib/kernel/qtmetamacros.h` / `tools/moc/moc.cpp` / `tools/moc/generator.cpp` / `plugin/qplugin.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 宏零展开注解 | qtmetamacros.h:56 | `#define Q_PLUGIN_METADATA(x) QT_ANNOTATE_CLASS(qt_plugin_metadata, x)` | 编译期空操作，全部作用是让 moc 看见。 |
| moc 解析 IID/URI/FILE 三键 | moc.cpp:1537-1599 | IID/URI/FILE 分支；FILE 读 JSON 文件 `metaData = file.readAll()` + `QJsonDocument::fromJson` | JSON 在 moc 阶段读入烧进二进制，故 metadata 不加载插件就能读。 |
| 【关键纠偏】Qt6 是 CBOR 非 JSON | generator.cpp:1408-1434 | `cbor_encoder_init_writer` / `cbor_encode_int(QtPluginMetaDataKeys::IID)` / `cbor_encode_text_string` | Qt5 是带 'QTMETADATA !' 前缀的 JSON 文本，Qt6 改 CBOR；JSON 只在 FILE 输入和 metaData() 输出两边界。 |
| 两枚 C 导出函数 | qplugin.h:240-254 | `extern "C" Q_DECL_EXPORT ... qt_plugin_query_metadata_v2()` + `qt_plugin_instance()` | extern C 无 mangling，插件跨编译器加载的根本原因。 |

## iid 校验（只在 QFactoryLoader）

源码文件：`qtbase/src/corelib/plugin/qpluginloader.cpp` / `qfactoryloader.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【重要纠偏】QPluginLoader 不校验 iid | qpluginloader.cpp（全文无 iid）+ qfactoryloader.cpp:337-345 | QPluginLoader 给定文件名即 load；iid 比对在 `QFactoryLoader::updateSinglePath` | QPluginLoader instance() 直接返回根 QObject 由调用方 qobject_cast。 |
| iid CBOR 流式扫描 | qfactoryloader.cpp:61-86+177-182 | `QFactoryLoaderIidSearch`：遍历到 IID 键比对→FinishedSearch 退出，其余 skip | 只读 IID 字段不解析整个 metadata，扫几十个 so 不 dlopen。 |

## metaData 返回

源码文件：`qtbase/src/corelib/plugin/qpluginloader.cpp` / `qplugin.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 对外 QJsonObject 内部 QCborMap | qpluginloader.cpp:156-161 | `return d->metaData.toJson();` | d->metaData 是 QPluginParsedMetaData（QCborMap）；toJson() 整数键转字符串。 |
| qt_plugin_query_metadata 返回 POD | qplugin.h:37-105 | `struct QPluginMetaData { ...; const void *data; size_t size; };` | 完整结构跨 37-105（含 Header/MagicString 嵌套），末尾 data+size 描述 CBOR 缓冲区。 |

## QFactoryLoader

源码文件：`qtbase/src/corelib/plugin/qfactoryloader.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 构造立刻 update() 扫描 | qfactoryloader.cpp:450-476 | `Q_ASSERT_X(suffix.startsWith(u'/')...)` + `update()` + 注册 qt_factoryloader_global | suffix 必须以 / 开头；注册进全局 loaders。 |
| 三层缓存 + PreventUnloadHint | qfactoryloader.cpp:262-264+293-393 | loadedPaths 去重 + libraries 容器 + keyMap；`setLoadHints(QLibrary::PreventUnloadHint) // once loaded, don't unload` | QFactoryLoader 管理的插件永远不卸载。 |
| 同 key 择优「宁老勿新」 | qfactoryloader.cpp:362-387 | 精确版本胜出；`existingVersion < QtVersionNoPatch && thisVersion > QtVersionNoPatch → continue // Better too old than too new` | 反直觉：宁可太老不要太新，避免新插件用老 Qt 没有的 API。 |

## 版本兼容检查

源码文件：`qtbase/src/corelib/plugin/qlibrary.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【关键纠偏】版本规则 | qlibrary.cpp:782-802 | `(qt_version & 0x00ff00) > (QT_VERSION & 0x00ff00) \|\| (qt_version & 0xff0000) != (QT_VERSION & 0xff0000)` | patch 不查；minor 用 > 非 !=（允许插件老，不允许插件新）；major 必须 ==。 |
| Windows debug/release 必须匹配 | qlibrary.cpp:44-49 | `PluginMustMatchQtDebug = (Windows && (非MinGW \|\| (MinGW && debug_and_release)))` | Unix 可混用 debug/release 插件。 |

## 静态插件

源码文件：`qtbase/src/corelib/plugin/qplugin.h` / `qpluginloader.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Q_IMPORT_PLUGIN 静态初始化注册 | qplugin.h:194-202 | 内部类构造调 `qRegisterStaticPluginFunction(qt_static_plugin_##PLUGIN())` + 文件级 static 实例 | main 前构造自动注册进 staticPluginList。 |
| 静态导出符号带 mangled | qplugin.h:222-228 | `static ... qt_plugin_instance_##MANGLEDNAME()` + `const QStaticPlugin qt_static_plugin_##MANGLEDNAME()` | static 不导出，靠 Q_IMPORT_PLUGIN 引用；与动态 extern C 裸名不同。 |
| staticInstances() 遍历 | qpluginloader.cpp:402-427 | `for (QStaticPlugin plugin : plugins) instances += plugin.instance();` | 静态插件不需 QPluginLoader 实例；QFactoryLoader 动态耗尽也扫静态。 |

## unload 与引用计数

源码文件：`qtbase/src/corelib/plugin/qlibrary_p.h` / `qlibrary.cpp` / `qpluginloader.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 双计数器 | qlibrary_p.h:115-118 | `QAtomicInt libraryRefCount;` + `QAtomicInt libraryUnloadCount;` | refCount 判能否 delete，unloadCount 判能否真卸载。 |
| 同文件复用同一 QLibraryPrivate | qlibrary.cpp:393-425 | `QLibraryStore::libraryMap` 按 fileName 去重；已存在 `libraryRefCount.ref()` | 两个 QPluginLoader 同文件底层是同一对象。 |
| 【关键纠偏】unload 须计数归零 | qlibrary.cpp:554-574 | `if (libraryUnloadCount > 0 && !libraryUnloadCount.deref()) { delete inst.data(); if (... unload_sys()) {...} }` | 注释 only unload if ALL QLibrary instance wanted to；卸载前 delete inst。macOS NoUnloadSys 不真卸载。 |
| 【重要纠偏】默认 PreventUnloadHint | qpluginloader.cpp:77+102 | `defaultLoadHints = QLibrary::PreventUnloadHint` | Unix→RTLD_NODELETE，Win→GetModuleHandleEx PIN；dlclose/FreeLibrary 是 no-op。想真卸载须清 hint。 |
| 析构只 release 不 unload | qpluginloader.cpp:113-117 | `~QPluginLoader() { if (d) d->release(); }` | 配合默认 hint，生产环境插件几乎从不真正卸载。 |

## 错误处理与不 dlopen 扫描

源码文件：`qtbase/src/corelib/plugin/qlibrary_unix.cpp` / `qlibrary.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| dlopen 失败用 dlerror() | qlibrary_unix.cpp:234-235 | `QLibrary::tr("Cannot load library %1: %2").arg(fileName, QString::fromLocal8Bit(dlerror()))` | 系统错误（undefined symbol 等）拼进 errorString。 |
| 版本不兼容错误列 Qt 版本 | qlibrary.cpp:790-795 | `"The plugin '%1' uses incompatible Qt library. (%2.%3.%4) [%5]"` | major.minor.patch + debug/release 标签。 |
| 找不到 metadata 入口=非 Qt 插件 | qlibrary.cpp:678-712 | 先 resolve `qt_plugin_query_metadata_v2`，退 `qt_plugin_query_metadata`，都失败 error | dlopen 成功不代表是 Qt 插件。Qt7 移除 v1 兼容。 |
| 【关键机制】findPatternUnloaded 不 dlopen | qlibrary.cpp:214-278 | `QFile::map` mmap + `qt_find_pattern` + `metaData.parse` | 平台 parser（ELF note/Mach-O/PE 段）定位 QTMETADATA；metaData() 不需先 load() 的根本机制。 |
| Q_PLUGIN_METADATA 隐式要 Q_OBJECT | moc.cpp:1050-1056 | 无 Q_OBJECT/Q_GADGET 且无 signals/slots/...→ `continue` 跳过；有 meta 需求才 error | 机制是「静默失效」非报错，开发者不易察觉。 |
