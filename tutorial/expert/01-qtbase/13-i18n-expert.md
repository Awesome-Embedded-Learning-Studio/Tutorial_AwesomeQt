---
title: QTranslator 与 QM 文件格式源码拆解
description: tr 凭啥是静态的、context 是编译期类名还是运行期的、tr 找不到翻译返回空还是原文、translators 列表后装的为啥优先、installTranslator 去不去重、QM 文件开头那 16 字节到底是啥、消息查找是哈希表还是二分、translation 字段是 UTF-8 还是 UTF-16、复数 %n 怎么走、disambiguation 和 comment 啥关系、QTranslator 析构会不会自摘除——从源码拆透。
---

# 现代Qt开发教程（专家篇）1.13——QTranslator 与 QM 文件格式源码拆解

## 1. 前言——国际化里几个答不上来的问题

Qt 的国际化，入门用法简单：源码里 `tr("Hello")`，`lupdate` 抽成 `.ts`，翻译完 `lrelease` 编成 `.qm`，运行时 `QTranslator::load` 加载、`installTranslator` 装上，「Hello」就变成「你好」。但这套流程底下藏着不少让人卡壳的细节。

笔者先把当年自己答不上来的几个问题摆出来。`tr()` 这个函数，您在 `QObject` 子类里直接写 `tr(...)`，它凭啥是静态的？`context` 这个参数到底是哪来的——是运行时的类名，还是编译期就定了的？`tr("Hello")` 在没装翻译的时候返回啥，空字符串还是原文？装多个 translator 时，谁优先？还有那个 `.qm` 文件，拿十六进制工具打开，开头那 16 个字节是什么，是可读的标记串还是二进制？最隐蔽的一个：消息查找到底是哈希表还是别的，教材都含糊。

这些问题，压在 Qt i18n 的几条主轴上：`tr` 的来源（`Q_OBJECT` 宏植入，context 是 moc 编译期烧进 `staticMetaObject` 的类名）、查找链（遍历 `translators` 列表，`prepend` 故后装优先，全 miss 返原文）、QM 文件格式（16 字节随机 magic、TLV 分节、无版本号字段）、查找算法（context 走哈希桶、消息走预排序数组二分）、内部编码（translation 是 UTF-16BE 不是 UTF-8）。

入门篇教了 `tr`/`QTranslator` 怎么用，进阶篇补了 `lupdate`/`lrelease` 流程和复数。本篇要往源码里捅：咱们打开 `qtranslator.cpp` 和 `qcoreapplication.cpp`，看看 `tr` 怎么一路走到 `QTranslator::translate`、`.qm` 文件二进制长啥样、消息是怎么被二分搜索找出来的。

边界先划清楚。`tr` 经 `staticMetaObject`（moc 生成）拿 context 的机制——`className()` 从元对象的 stringdata 表里取类名——咱们只在「编译期烧入类名」这个层面点到，`staticMetaObject` 的完整七字段结构是 [1.01 QObject 元对象系统篇](./01-qobject-meta-system-expert.md) 和 [17.MOC 编译器原理篇](./17-moc-compiler-expert.md) 的范畴。`lupdate`/`lrelease` 工具（`.ts` 转 `.qm`、XML 解析、字节码生成）是工具链主题，本篇讲的是运行时 `QTranslator` 怎么读 `.qm` 查找，不讲工具怎么生成 `.qm`。具体应用的 `retranslateUi` 实践只点到 `installTranslator` 发 `LanguageChange` 事件触发，不展开 UI 重译模式。

## 2. 环境说明

本篇源码引用基于 `qt_src/qt6.9.1`，行号随 Qt 版本会漂移，对照阅读时拿函数名定位最稳。i18n 涉及的关键文件：

| 文件 | 角色 |
|---|---|
| `qtbase/src/corelib/kernel/qtranslator.h` | QTranslator 公共声明 |
| `qtbase/src/corelib/kernel/qtranslator.cpp` | QM 格式解析 + translate 查找算法（1143 行） |
| `qtbase/src/corelib/kernel/qtranslator_p.h` | 复数规则操作码常量（私有头） |
| `qtbase/src/corelib/kernel/qcoreapplication.cpp` | translate / installTranslator / removeTranslator / replacePercentN |
| `qtbase/src/corelib/kernel/qcoreapplication_p.h` | translators 列表 + translateMutex |
| `qtbase/src/corelib/kernel/qtmetamacros.h` | QT_TR_FUNCTIONS / Q_OBJECT 宏 |
| `qtbase/src/corelib/kernel/qmetaobject.cpp` | QMetaObject::tr / className |
| `qtbase/src/corelib/global/qttranslation.h` | QT_TR_NOOP / qtTrId 等 |

本篇无配套 example，原因和前几篇一样：纯源码拆解，对照 `qt_src` 翻代码就是最好的实验。

## 3. 核心概念讲解

下源码之前，咱们先把 `tr` 从源码到翻译结果的完整链路对一下。这张图能帮您看清中间发生了什么：

```mermaid
flowchart TD
    A["MyClass::tr(\"Hello\")"] -->|"inline static"| B["staticMetaObject.tr(...)"]
    B -->|"QMetaObject::tr"| C["QCoreApplication::translate(className(), ...)"]
    C -->|"className 从 stringdata 取"| D["context = 'MyClass'"]
    D --> E{"遍历 translators 列表"}
    E -->|"每个 QTranslator::translate"| F{命中?}
    F -->|"是,非 null"| G["返回译文 + replacePercentN"]
    F -->|"全 miss"| H["返回 QString::fromUtf8(sourceText) 原文"]
    G --> I["调用方拿到结果"]
    H --> I
```

`tr` 是 `Q_OBJECT` 宏植入的 inline static，转发给 `staticMetaObject.tr`，后者把 context 填成 moc 编译期烧入的类名，交给 `QCoreApplication::translate` 遍历所有已安装的 `QTranslator`。命中返回译文，全 miss 返回原文。咱们这一篇就顺着这条链拆。

### 3.1 tr 是 Q_OBJECT 植入的，context 是编译期类名

先看 `tr` 到底从哪来。很多人以为 `QObject::tr` 是 `QObject` 类自己定义的方法——不是。它是由 `Q_OBJECT` 宏植入的：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtmetamacros.h:84-92`

```cpp
#ifndef QT_NO_TRANSLATION
// full set of tr functions
#  define QT_TR_FUNCTIONS \
    static inline QString tr(const char *s, const char *c = nullptr, int n = -1) \
        { return staticMetaObject.tr(s, c, n); }
#else
// inherit the ones from QObject
# define QT_TR_FUNCTIONS
#endif
```

`QT_TR_FUNCTIONS` 宏展开成一个 inline static 函数，转发给 `staticMetaObject.tr`。这个宏被 `Q_OBJECT` 宏塞进每个 `QObject` 子类的类体：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtmetamacros.h:133-146`

```cpp
#define Q_OBJECT \
public: \
    QT_WARNING_PUSH \
    Q_OBJECT_NO_OVERRIDE_WARNING \
    static const QMetaObject staticMetaObject; \
    virtual const QMetaObject *metaObject() const; \
    virtual void *qt_metacast(const char *); \
    virtual int qt_metacall(QMetaObject::Call, int, void **); \
    QT_TR_FUNCTIONS \
private: \
    ...
```

所以每个声明了 `Q_OBJECT` 的类，都有自己的 `tr`——它不是继承来的，是宏在类体里展开出来的。这带来一个直接后果：没声明 `Q_OBJECT` 的类，压根就没有 `tr`，调它会编译失败。

`tr` 转发到 `QMetaObject::tr`，这里有个关键动作——填 context：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qmetaobject.cpp:416-419`

```cpp
QString QMetaObject::tr(const char *s, const char *c, int n) const
{
    return QCoreApplication::translate(className(), s, c, n);
}
```

context 不是运行时的类名，是 `className()` 返回的——而 `className()` 是从 `staticMetaObject` 的 stringdata 表里按索引取出来的字符串，那个字符串是 moc 在编译期写进去的类名。换句话说，您写 `MyClass::tr("Hello")`，最终 `translate` 收到的 context 是字符串 `"MyClass"`，这个绑定在编译期就完成了，运行时改不了。

非 `QObject` 派生的类（工具类、命名空间）也想用 `tr` 怎么办？用 `Q_DECLARE_TR_FUNCTIONS`：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.h:257-261`

```cpp
#define Q_DECLARE_TR_FUNCTIONS(context) \
public: \
    static inline QString tr(const char *sourceText, const char *disambiguation = nullptr, int n = -1) \
        { return QCoreApplication::translate(#context, sourceText, disambiguation, n); } \
private:
```

宏参数 `context` 被 `#context` 字符串化，直接传给 `translate`，绕过 `staticMetaObject`。这是给非 `QObject` 类开的口子。

还有个边角：如果编译时定义了 `QT_NO_TRANSLATION`（完全禁用翻译），`tr` 还在吗？在，但退化了：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qobject.h:119-122`

```cpp
#if defined(QT_NO_TRANSLATION) || defined(Q_QDOC)
    static QString tr(const char *sourceText, const char * = nullptr, int = -1)
        { return QString::fromUtf8(sourceText); }
#endif
```

`tr` 直接返回 `QString::fromUtf8(sourceText)`，啥也不查。所以您代码里那些 `tr(...)` 调用，禁用翻译时不用 `#ifdef` 包裹，照样能编译、能跑，只是不翻译。

### 3.2 translate 遍历 translators 列表，全 miss 返原文

接下来是本篇第一个大纠偏点。`tr` 找不到翻译时返回什么——空字符串？笔者当年也是这么以为的，错。

看 `QCoreApplication::translate` 的实现：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.cpp:2283-2311`

```cpp
QString QCoreApplication::translate(const char *context, const char *sourceText,
                                    const char *disambiguation, int n)
{
    QString result;

    if (!sourceText)
        return result;

    if (self) {
        QCoreApplicationPrivate *d = self->d_func();
        QReadLocker locker(&d->translateMutex);
        if (!d->translators.isEmpty()) {
            QList<QTranslator*>::ConstIterator it;
            QTranslator *translationFile;
            for (it = d->translators.constBegin(); it != d->translators.constEnd(); ++it) {
                translationFile = *it;
                result = translationFile->translate(context, sourceText, disambiguation, n);
                if (!result.isNull())
                    break;
            }
        }
    }

    if (result.isNull())
        result = QString::fromUtf8(sourceText);

    replacePercentN(&result, n);
    return result;
}
```

逻辑分两段。第一段遍历 `translators` 列表，对每个已安装的 `QTranslator` 调它的 `translate`，命中（`result.isNull()` 为 false）就 break。第二段是关键兜底——如果遍历完还是 null，`result = QString::fromUtf8(sourceText)`，返回的是源文本的 UTF-8 解码，也就是英文原文。

所以 `tr` 找不到翻译，返回的不是空字符串，是源文本本身。这就是为啥应用即便没装任何翻译，界面也能正常显示英文——`tr("Hello")` 在没 translator 时返回 `"Hello"`。只有 `sourceText == nullptr` 这个极端情况才返回空 `QString`。

末尾那个 `replacePercentN(&result, n)` 是处理复数里的 `%n` 占位符的，3.7 节细讲。

### 3.3 installTranslator：后装优先，不去重

`translators` 列表怎么维护的，决定了多个 translator 谁先查。看 `installTranslator`：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.cpp:2165-2176`

```cpp
bool QCoreApplication::installTranslator(QTranslator *translationFile)
{
    if (!translationFile)
        return false;

    if (!QCoreApplicationPrivate::checkInstance("installTranslator"))
        return false;
    QCoreApplicationPrivate *d = self->d_func();
    {
        QWriteLocker locker(&d->translateMutex);
        d->translators.prepend(translationFile);
    }
```

`prepend`——放列表头部。结合 3.2 节的 `constBegin → constEnd` 正向遍历，列表头部是最先被查的。所以「后装的 translator 优先级更高」。官方文档说的「queries in reverse installation order」（逆安装顺序查询）就是这么来的：安装顺序是 A→B→C，列表变成 C B A，查询顺序就是 C B A。

这里有个坑，笔者专门拎出来。`installTranslator` 只做了 nullptr 检查和 instance 检查，没有 `contains` 去重。也就是说，同一个 translator 您 install 两次，列表里会出现两条。虽然查询时因为幂等返回结果一样，但这是无意义的重复，而且 `removeTranslator` 用的 `removeAll` 会一次删掉所有匹配项——重复 install 会扰乱您的「装一次卸一次」心智模型。

`removeTranslator` 的实现：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.cpp:2201-2220`

```cpp
bool QCoreApplication::removeTranslator(QTranslator *translationFile)
{
    if (!translationFile)
        return false;
    if (!QCoreApplicationPrivate::checkInstance("removeTranslator"))
        return false;
    QCoreApplicationPrivate *d = self->d_func();
    QWriteLocker locker(&d->translateMutex);
    if (d->translators.removeAll(translationFile)) {
#ifndef QT_NO_QOBJECT
        locker.unlock();
        if (!self->closingDown()) {
            QEvent ev(QEvent::LanguageChange);
            QCoreApplication::sendEvent(self, &ev);
        }
#endif
        return true;
    }
    return false;
}
```

`removeAll` 删全部匹配项（一次清掉重复 install 的所有条目）。返回 bool 表示是否实际移除，没找到不发 `LanguageChange` 事件。注意 `installTranslator` 成功后和 `removeTranslator` 成功后都会 `sendEvent` 一个 `LanguageChange`（同步派发），这是 UI 动态重译的信号源——`QApplication` 把它派发给顶层 widget，触发它们的 `changeEvent`，您在 `changeEvent` 里重写 `retranslateUi` 就能动态切换语言。

`translators` 列表本身受读写锁保护：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication_p.h:158-162`

```cpp
#ifndef QT_NO_TRANSLATION
    QTranslatorList translators;
    QReadWriteLock translateMutex;
    static bool isTranslatorInstalled(QTranslator *translator);
#endif
```

`translate` 用 `QReadLocker`（多线程可并发查），`installTranslator`/`removeTranslator` 用 `QWriteLocker`（独占写）。翻译查询是多线程安全的。

### 3.4 QM 文件：16 字节随机 magic，无版本号

重头戏来了。咱们打开 `qtranslator.cpp`，看看 `.qm` 文件二进制到底长啥样。

第一个反直觉的地方：`.qm` 文件开头那 16 个字节，很多人以为是可读的标记串（比如 `<QTMETADATA>` 之类）。笔者第一次 hexdump 一个 qm 文件时也找过可读头，没找着——它是随机二进制：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:51-62`

```cpp
/*
$ mcookie
3cb86418caef9c95cd211cbf60a1bddd
$
*/

// magic number for the file
static const int MagicLength = 16;
static const uchar magic[MagicLength] = {
    0x3c, 0xb8, 0x64, 0x18, 0xca, 0xef, 0x9c, 0x95,
    0xcd, 0x21, 0x1c, 0xbf, 0x60, 0xa1, 0xbd, 0xdd
};
```

注释里那段 `mcookie` 命令的输出——`3cb86418caef9c95cd211cbf60a1bddd`——就是这 16 字节的来源。`mcookie` 是 Unix 下生成随机 cookie 的工具，Qt 当年拿它随机生成了这串字节，固化成了 QM 文件的 magic number。所以这 16 字节没有任何语义，纯粹是随机数当文件标识。

第二个反直觉的地方：`.qm` 文件没有显式的版本号字段。magic 之后直接就是一串「tag-length-value」结构：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:260`

```cpp
    enum { Contexts = 0x2f, Hashes = 0x42, Messages = 0x69, NumerusRules = 0x88, Dependencies = 0x96, Language = 0xa7 };
```

这是 section tag 枚举。`do_load` 的解析循环：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:794-836`（节选）

```cpp
bool QTranslatorPrivate::do_load(const uchar *data, qsizetype len, const QString &directory)
{
    bool ok = true;
    const uchar *end = data + len;

    data += MagicLength;
    ...
    while (data < end - 5) {
        quint8 tag = read8(data++);
        quint32 blockLen = read32(data);
        data += 4;
        if (!tag || !blockLen)
            break;
        if (quint32(end - data) < blockLen) {
            ok = false;
            break;
        }

        if (tag == QTranslatorPrivate::Language) {
            language = QString::fromUtf8((const char *)data, blockLen);
        } else if (tag == QTranslatorPrivate::Contexts) {
            contextArray = data;
            contextLength = blockLen;
        } else if (tag == QTranslatorPrivate::Hashes) {
            offsetArray = data;
            offsetLength = blockLen;
        } else if (tag == QTranslatorPrivate::Messages) {
            messageArray = data;
            messageLength = blockLen;
        } else if (tag == QTranslatorPrivate::NumerusRules) {
            numerusRulesArray = data;
            numerusRulesLength = blockLen;
        } else if (tag == QTranslatorPrivate::Dependencies) {
```

跳过 16 字节 magic 后，进入 TLV 循环：1 字节 tag（`read8`）+ 4 字节大端长度（`read32`）+ 数据。识别到的 tag 存进对应字段，未识别的 tag 直接跳过（前向兼容——未来加新 section，老版本 Qt 能跳过不认识的部分）。整个格式没有版本号字段，Qt5 和 Qt6 的 `.qm` 用同一套 magic + section tag，文件级兼容。

注意那个命名错位：`Hashes` 这个 section 在内存里对应的是 `offsetArray`/`offsetLength`，存的是「hash → 消息偏移」的二元组。为啥叫 Hashes 又存 offset？历史遗留，您看源码时要心里有数。

`load` 加载时第一件事就是校验 magic：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:539-544`

```cpp
        {
            char magicBuffer[MagicLength];
            if (MagicLength != file.read(magicBuffer, MagicLength)
                    || memcmp(magicBuffer, magic, MagicLength))
                return false;
        }
```

长度不足 16 或 magic 不符，立即返回 false，文件根本不会加载。

加载方式上，Unix 平台优先用 `mmap` 把整个 `.qm` 映射进内存：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:548-581`（节选）

```cpp
        int fd = file.handle();
        if (fd >= 0) {
            int protection = PROT_READ;                 // read-only memory
            int flags = MAP_FILE | MAP_PRIVATE;         // swap-backed map from file
            void *ptr = QT_MMAP(nullptr, d->unmapLength,
                                protection, flags,
                                fd, 0);
            if (ptr != MAP_FAILED) {
                file.close();
                d->used_mmap = true;
                d->unmapPointer = static_cast<char *>(ptr);
                ok = true;
            }
        }
        ...
        if (!ok) {
            d->unmapPointer = new (std::nothrow) char[d->unmapLength];
            ...
            qint64 readResult = file.read(d->unmapPointer, d->unmapLength);
        }
```

`PROT_READ + MAP_PRIVATE` 只读私有映射，翻译查询零拷贝——直接在映射的内存上跑查找算法。mmap 失败才回退到 `new` + `read`。如果 `.qm` 在 Qt 资源系统里且未压缩，连 mmap 都省了，直接复用 `QResource::data()` 的指针：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:511-527`

```cpp
    if (realname.startsWith(u':')) {
        // If the translation is in a non-compressed resource file, the data is already in
        // memory, so no need to use QFile to copy it again.
        Q_ASSERT(!d->resource);
        d->resource = std::make_unique<QResource>(realname);
        if (resource->isValid() && resource->compressionAlgorithm() == QResource::NoCompression
                && resource->size() >= MagicLength
                && !memcmp(resource->data(), magic, MagicLength)) {
            d->unmapLength = resource->size();
            d->unmapPointer = reinterpret_cast<char *>(const_cast<uchar *>(resource->data()));
            ...
            ok = true;
        }
    }
```

资源内未压缩的 QM，直接拿 `QResource::data()` 的指针用，零拷贝。

### 3.5 查找算法：context 走哈希桶，消息走二分

`.qm` 加载进内存后，`translate` 怎么按 (context, sourceText) 找到译文？这是本篇最有「源码含金量」的部分。

先看 context 查找。`do_translate` 第一步在 `Contexts` section 里定位 context：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:957-976`

```cpp
    if (contextLength) {
        quint16 hTableSize = read16(contextArray);
        uint g = elfHash(context) % hTableSize;
        const uchar *c = contextArray + 2 + (g << 1);
        quint16 off = read16(c);
        c += 2;
        if (off == 0)
            return QString();
        c = contextArray + (2 + (hTableSize << 1) + (off << 1));

        const uint contextLen = uint(strlen(context));
        for (;;) {
            quint8 len = read8(c++);
            if (len == 0)
                return QString();
            if (match(c, len, context, contextLen))
                break;
            c += len;
        }
    }
```

`contextArray` 前 2 字节是桶数 `hTableSize`，算 `elfHash(context) % hTableSize` 落到某个桶。桶里存的是偏移，顺着偏移跳到桶链，然后线性扫——读一个长度、比一次字符串，匹配上就 break，读到长度 0（链尾）还没匹配就返回空。这是个「哈希 + 桶内线性探测」的结构。关键优化：如果 context 在本 QM 里压根不存在，这里立即 `return QString()`，不浪费后续的消息查找。

消息查找（在 `Hashes`/offsetArray 里）用的是另一套算法——二分搜索，不是哈希桶表：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:985-1023`（节选）

```cpp
    for (;;) {
        quint32 h = 0;
        elfHash_continue(sourceText, h);
        elfHash_continue(comment, h);
        elfHash_finish(h);

        const uchar *start = offsetArray;
        const uchar *end = start + ((numItems - 1) << 3);
        while (start <= end) {
            const uchar *middle = start + (((end - start) >> 4) << 3);
            uint hash = read32(middle);
            if (h == hash) {
                start = middle;
                break;
            } else if (hash < h) {
                start = middle + 8;
            } else {
                end = middle - 8;
            }
        }

        if (start <= end) {
            // go back on equal key
            while (start != offsetArray && read32(start) == read32(start - 8))
                start -= 8;

            while (start < offsetArray + offsetLength) {
                quint32 rh = read32(start);
                start += 4;
                if (rh != h)
                    break;
                quint32 ro = read32(start);
                start += 4;
                QString tn = getMessage(messageArray + ro, messageArray + messageLength, context,
                                        sourceText, comment, numerus);
                if (!tn.isNull())
                    return tn;
            }
        }
        if (!comment[0])
            break;
        comment = "";
    }
```

这是本篇第二个大纠偏点。笔者翻过的教材笼统说「QM 用哈希表查找」，不准确。实际算法是：`offsetArray` 是一个按 hash 升序预排序的数组，每项 8 字节（hash uint32 + offset uint32）。`while (start <= end)` 那段是教科书式的二分搜索——`middle` 取中点，比 hash 大小调整 `start`/`end`。命中 hash 后，先回头扫所有相同 hash 的项（处理哈希碰撞），逐项调 `getMessage` 验证 sourceText/context/comment 真匹配（哈希只用来缩小范围，不保证唯一）。

注意 hash 的计算：

```cpp
        quint32 h = 0;
        elfHash_continue(sourceText, h);
        elfHash_continue(comment, h);
        elfHash_finish(h);
```

`h` 从 0 开始，先用 `elfHash_continue` 把 sourceText 的字节流喂进去，再继续把 comment 的字节流喂进去（接在同一个哈希状态机上），最后 `elfHash_finish` 收尾。这是「累加哈希」，不是「sourceText 和 comment 各算一个哈希再异或」。`elfHash_continue` 的实现：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:75-101`

```cpp
static void elfHash_continue(const char *name, uint &h)
{
    const uchar *k;
    uint g;

    k = (const uchar *) name;
    while (*k) {
        h = (h << 4) + *k++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 24;
        h &= ~g;
    }
}
```

4 位左移、高 4 位异或——这就是经典的 Unix ELF 符号表哈希（所以叫 elfHash）。`_continue` 设计成可拼接，让 (sourceText, comment) 组合产生单一哈希键。

外层那个 `for (;;)` 跑两轮：第一轮用原 comment 查；查不到就把 `comment` 清空跑第二轮，让带 disambiguation 的查询也能命中没填 disambiguation 的译文。这是 3.7 节要讲的 disambiguation 回退。

### 3.6 哈希命中后的精确匹配：getMessage 变长 TLV

二分找到匹配的 hash 后，光有 hash 不够——哈希会碰撞。`getMessage` 在 `Messages` section 里解析变长记录做精确匹配：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:870-935`（节选）

```cpp
static QString getMessage(const uchar *m, const uchar *end, const char *context,
                          const char *sourceText, const char *comment, uint numerus)
{
    const uchar *tn = nullptr;
    uint tn_length = 0;
    const uint sourceTextLen = uint(strlen(sourceText));
    const uint contextLen = uint(strlen(context));
    const uint commentLen = uint(strlen(comment));

    for (;;) {
        uchar tag = 0;
        if (m < end)
            tag = read8(m++);
        switch ((Tag)tag) {
        case Tag_End:
            goto end;
        case Tag_Translation: {
            int len = read32(m);
            if (len & 1)
                return QString();
            m += 4;
            if (!numerus--)
                tn_length = len;
                tn = m;
            }
            m += len;
            break;
        }
        ...
        case Tag_SourceText: {
            quint32 len = read32(m);
            m += 4;
            if (!match(m, len, sourceText, sourceTextLen))
                return QString();
            m += len;
        }
            break;
        ...
        default:
            return QString();
        }
    }
end:
    if (!tn)
        return QString();
    QString str(tn_length / 2, Qt::Uninitialized);
    qFromBigEndian<char16_t>(tn, str.size(), str.data());
    return str;
}
```

变长 TLV 结构：每条记录由若干个 `Tag` 开头的字段组成（Tag_Translation / Tag_SourceText / Tag_Context / Tag_Comment / Tag_End）。每个字段带一个 4 字节长度，后面跟数据。`match` 函数做精确字节比对，任何一个字段不匹配立即返回空。这层精确匹配保证了哈希碰撞不会误命中。

这里有个本篇第三个大纠偏点，也是笔者手动解析 qm 时踩过的坑——translation 字段的编码。看末尾这几行：

```cpp
    QString str(tn_length / 2, Qt::Uninitialized);
    qFromBigEndian<char16_t>(tn, str.size(), str.data());
    return str;
```

`tn_length / 2`——字节数除以 2 得字符数。`qFromBigEndian<char16_t>`——把大端字节流的 16 位单元转成 host-endian 的 char16_t，填进 `QString`。这说明 translation 字段在 `.qm` 里是 UTF-16 大端编码，不是 UTF-8。

往上翻那个校验也印证了这点：

```cpp
        case Tag_Translation: {
            int len = read32(m);
            if (len & 1)
                return QString();
```

`if (len & 1) return QString()`——长度必须是偶数，因为 UTF-16 的单位是 2 字节。奇数长度直接判定非法。

所以 `.qm` 内部编码是不统一的：sourceText、comment、context 这些在 `lupdate` 阶段以 UTF-8 处理（存进 QM 时也是 UTF-8 字节流，靠 `strlen`/`match` 当裸字节比）；Language section（语言代码短串）用 `QString::fromUtf8` 解码；唯独 translation 字段是 UTF-16 大端。您要是手动解析 `.qm`，得记住这个区别。

### 3.7 复数 %n：字节码虚拟机

复数处理是 i18n 里绕不开的话题。笔者第一次知道 Qt 的复数规则存成字节码时还挺意外——`.qm` 的 `NumerusRules` section 里是一段字节码，`QTranslator` 用一个小型栈式虚拟机执行它。

`translate` 拿到 n 后先算出复数形式索引：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:982-983`

```cpp
    if (n >= 0)
        numerus = numerusHelper(n, numerusRulesArray, numerusRulesLength);
```

`numerusHelper` 执行字节码：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:177-254`（节选）

```cpp
static uint numerusHelper(int n, const uchar *rules, uint rulesSize)
{
    uint result = 0;
    uint i = 0;

    if (rulesSize == 0)
        return 0;

    for (;;) {
        ...
        for (;;) {
            ...
            for (;;) {
                bool truthValue = true;
                int opcode = rules[i++];

                int leftOperand = n;
                if (opcode & Q_MOD_10) {
                    leftOperand %= 10;
                } else if (opcode & Q_MOD_100) {
                    leftOperand %= 100;
                } else if (opcode & Q_LEAD_1000) {
                    while (leftOperand >= 1000)
                        leftOperand /= 1000;
                }

                int op = opcode & Q_OP_MASK;
                int rightOperand = rules[i++];
                ...
        if (orExprTruthValue)
            return result;

        ++result;

        if (i == rulesSize)
            return result;

        i++; // Q_NEWRULE
    }
```

操作码是位掩码设计的：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator_p.h:20-41`

```cpp
enum {
    Q_EQ          = 0x01,
    Q_LT          = 0x02,
    Q_LEQ         = 0x03,
    Q_BETWEEN     = 0x04,

    Q_NOT         = 0x08,
    Q_MOD_10      = 0x10,
    Q_MOD_100     = 0x20,
    Q_LEAD_1000   = 0x40,

    Q_AND         = 0xFD,
    Q_OR          = 0xFE,
    Q_NEWRULE     = 0xFF,

    Q_OP_MASK     = 0x07,

    Q_NEQ         = Q_NOT | Q_EQ,
    Q_GT          = Q_NOT | Q_LEQ,
    Q_GEQ         = Q_NOT | Q_LT,
    Q_NOT_BETWEEN = Q_NOT | Q_BETWEEN
};
```

低 3 位是操作（EQ/LT/LEQ/BETWEEN），高位是修饰（MOD_10 取个位、MOD_100 取百位、LEAD_1000 取千位前导、NOT 取反）。虚拟机执行时，`Q_MOD_10` 这类修饰会先变换左操作数（n），再用低 3 位的操作跟右操作数比较。`Q_AND`/`Q_OR` 组合逻辑表达式，`Q_NEWRULE` 分隔多条规则。

这套字节码能表达各语言的复数规则。英语就两条规则：「n==1 用 singular（索引 0），其余用 plural（索引 1）」。阿拉伯语复杂得多，有「一个」「两个」「少数」「多数」等多种形式，对应多条 `Q_NEWRULE`。`result` 累加命中的规则索引，回传给 `getMessage` 当 `numerus` 用——`getMessage` 里的 `if (!numerus--)` 是个倒计数器，逐个跳过 `Tag_Translation` 字段，到第 `numerus` 个取出来。

最终 `%n` 占位符的替换发生在 `translate` 末尾的 `replacePercentN`：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.cpp:2222-2248`（节选）

```cpp
static void replacePercentN(QString *result, int n)
{
    if (n >= 0) {
        qsizetype percentPos = 0;
        qsizetype len = 0;
        while ((percentPos = result->indexOf(u'%', percentPos + len)) != -1) {
            len = 1;
            ...
            if (result->at(percentPos + len) == u'n') {
                fmt = fmt.arg(n);
                ++len;
                result->replace(percentPos, len, fmt);
                len = fmt.size();
            }
        }
    }
}
```

扫译文里的 `%n` 替换成 n 的字符串，`%Ln` 用本地化格式（千分位）。所以 Qt 复数翻译的标准写法是 `tr("%n file(s)", nullptr, count)`——`count` 决定复数形式（经 `numerusHelper`），译文里的 `%n` 再被替换成实际数字。

### 3.8 disambiguation、依赖链、析构自摘除

最后补几个零散但重要的点。

disambiguation（第三参数，Qt5+ 新名；Qt4 叫 comment）的作用是区分「同名 sourceText 在不同场景」。它同时进哈希键和精确匹配：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:986-989,917-923`

```cpp
// 哈希
        quint32 h = 0;
        elfHash_continue(sourceText, h);
        elfHash_continue(comment, h);
        elfHash_finish(h);
...
// 精确匹配
        case Tag_Comment: {
            quint32 len = read32(m);
            m += 4;
            if (*m && !match(m, len, comment, commentLen))
                return QString();
            m += len;
        }
            break;
```

所以「`tr("Open", "File menu")`」和「`tr("Open", "Disk operation")`」会得到不同的 hash、落到不同的桶、被正确区分。注意精确匹配里 `if (*m && ...)` 那个条件——如果 QM 里存的 comment 是空串（首字节 0），不强制匹配，允许「没填 disambiguation 的译文」匹配「查询时带了 disambiguation 的请求」。这是 3.5 节外层 `for (;;)` 两轮回退的配合机制。

`.qm` 还能声明依赖其他 `.qm`（`Dependencies` section，`lrelease` 时指定）：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:1029-1035`

```cpp
searchDependencies:
    for (const auto &translator : subTranslators) {
        QString tn = translator->translate(context, sourceText, comment, n);
        if (!tn.isNull())
            return tn;
    }
    return QString();
```

主 translator 没命中时，向它加载的子 translator（`subTranslators`）转发查询。加载时递归 `load` 所有依赖，任一失败全部清理（fail-fast）。这机制常用于「基础语言包 + 地区变体」的场景。

最后看 `QTranslator` 析构——这是个容易踩的坑：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:406-412`

```cpp
QTranslator::~QTranslator()
{
    if (QCoreApplication::instance())
        QCoreApplication::removeTranslator(this);
    Q_D(QTranslator);
    d->clear();
}
```

析构时主动调 `removeTranslator` 把自己从 `QCoreApplication` 摘除。这是因为 `QCoreApplication` 不拥有 `QTranslator` 的所有权（doc 明确写 `does not take ownership`），但 translator 装进去后 `QCoreApplication` 持有它的裸指针——如果 translator 先于 `QCoreApplication` 析构，那个指针就悬空了。所以 `~QTranslator` 自摘除防悬挂（前提是 `QCoreApplication` 还活着）。这也是为啥 translator 通常建议做成栈对象或挂到对象树上——靠生命周期自动管理摘除。

## 4. 踩坑预防

本篇踩坑只讲源码里能直接对应、笔者自己也栽过的真坑。

### 4.1 tr 找不到翻译返回原文，不是空串

后果：您用 `if (tr("Hello").isEmpty())` 判断「翻译是否生效」，永远进不去分支——因为 `tr` 找不到时返回的是 `"Hello"` 本身，不是空串。整个应用看似翻译功能正常（显示英文），其实您的 translator 根本没加载成功，您却以为加载了。

根因是 3.2 节的兜底逻辑：`if (result.isNull()) result = QString::fromUtf8(sourceText);`。只有 `sourceText == nullptr` 才返空 `QString`，正常的 miss 都返原文。

正确做法：判断 translator 是否生效，查 `QTranslator::isEmpty()`（加载后是否真有内容），或者直接看界面文案有没有变。别用 `tr(...).isEmpty()` 当判据。

### 4.2 installTranslator 不去重，重复 install 留重复条目

后果：您在某个槽函数里每次响应语言切换都 `installTranslator(myTranslator)`，以为「重新装一下刷新」。实际列表里这个 translator 越积越多（每次 prepend 一条），虽然查询幂等结果不变，但遍历 `translators` 列表越来越长，每次 `translate` 都要白白多查几遍。更糟的是您以为「卸载一次」能清掉，结果 `removeAll` 一次删光，行为和预期不符。

根因是 3.3 节讲的：`installTranslator` 只 prepend 不去重。

正确做法：translator 全局只 install 一次。要切换语言，先 `removeTranslator` 旧的、`load` 新 QM 到同一个 translator 对象、再靠 `LanguageChange` 事件触发刷新；或者 remove 旧的再 install 新的，别对同一个对象反复 install。

### 4.3 多个 translator 的优先级：后装的先查

后果：您装了两个 translator（A 通用翻译、B 行业术语补丁），期望 A 兜底、B 覆盖特定词条。结果发现 B 的覆盖没生效，查到的是 A 的译文。或者反过来，您不知道谁覆盖谁，翻译结果不稳定。

根因是 3.3 节的 prepend + 正向遍历：列表头部是最新装的，最先被查。安装顺序 A→B，查询顺序 B→A，B 命中就返回，不会查 A。

正确做法：明确「后装的优先级更高」。想让补丁 translator 覆盖基础 translator，补丁必须后 install。需要调整优先级，先 remove 再按期望顺序重新 install（最后装的最优先）。

### 4.4 没有 Q_OBJECT 的类没有 tr

后果：您在一个没继承 `QObject` 的工具类里写 `tr("Hello")`，编译报错「no member named 'tr'」。或者类继承了 `QObject` 但忘了 `Q_OBJECT` 宏，调 `tr` 编译失败。

根因是 3.1 节讲的：`tr` 是 `Q_OBJECT` 宏植入的，没 `Q_OBJECT` 就没 `tr`。

正确做法：`QObject` 派生类必须 `Q_OBJECT` 宏；非 `QObject` 类（工具类、命名空间）用 `Q_DECLARE_TR_FUNCTIONS(ClassName)` 宏注入 `tr`，context 由宏参数字符串化得到。

## 5. 官方文档参考链接

- [QTranslator Class Reference](https://doc.qt.io/qt-6/qtranslator.html) —— load/translate/isEmpty
- [QCoreApplication::translate](https://doc.qt.io/qt-6/qcoreapplication.html#translate) —— context/disambiguation/n 参数与找不到返原文行为
- [QCoreApplication::installTranslator](https://doc.qt.io/qt-6/qcoreapplication.html#installTranslator) —— prepend 不去重 + 后装优先
- [Qt Linguist Manual](https://doc.qt.io/qt-6/linguist-manual.html) —— lupdate/lrelease 工具链
- [Writing Source Code for Translation](https://doc.qt.io/qt-6/i18n-source-translation.html) —— tr/Q_DECLARE_TR_FUNCTIONS/qtTrId 使用规范
- [Plural Form Support](https://doc.qt.io/qt-6/i18n-source-translation.html#plural-form) —— %n 与 NumerusRules 复数规则

---

Qt 的国际化设计哲学，是「源码无感 + 运行时按需查找」——您源码里写 `tr`，编译期 moc 把类名烧进 `staticMetaObject` 当 context，运行时 `QCoreApplication` 遍历 translator 列表，每个 `QTranslator` 在 `.qm` 的二进制结构里做哈希+二分查找。这套设计让源码和翻译资源彻底解耦，切换语言不用重编程序。但它也藏着不少反直觉的细节：`tr` 找不到返原文、后装的优先、不去重、QM 用随机 magic、消息走二分不是哈希表、translation 是 UTF-16 而 sourceText 是 UTF-8。这些细节平时用不到，可一旦要做翻译调试、性能优化、或者手动解析 QM，就是必须懂的底层。读完这篇，您应该能在翻译行为诡异时，知道往哪儿查了。
