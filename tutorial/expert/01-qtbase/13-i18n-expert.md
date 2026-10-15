---
title: QTranslator 与 QM 文件格式源码拆解
description: tr 凭啥是静态的、context 是编译期类名还是运行期的、tr 找不到翻译返回空还是原文、translators 列表后装的为啥优先、installTranslator 去不去重、QM 文件开头那 16 字节到底是啥、消息查找是哈希表还是二分、translation 字段是 UTF-8 还是 UTF-16、复数 %n 怎么走、disambiguation 和 comment 啥关系、QTranslator 析构会不会自摘除——从源码拆透。
---

# QTranslator 与 QM 文件格式源码拆解

## 前言：国际化里几个答不上来的问题

Qt 的国际化，入门用法是不复杂的：咱们在源码里写 `tr("Hello")`，`lupdate` 把字符串抽成了 `.ts`，翻译完了 `lrelease` 把它编成了 `.qm`，运行时咱们拿 `QTranslator::load` 加载、`installTranslator` 装上，“Hello”就变成了“你好”。不过这套流程底下藏着不少能让人卡壳的细节，咱们这一篇就是冲着它们去的。

笔者把当年自己答不上来的问题摆出来。`tr()` 凭啥是个静态的函数？您在 `QObject` 子类里直接写 `tr(...)`，写的时候没加任何 static，它凭啥是静态的？`context` 这个参数是哪来的，是运行时的类名，还是编译期就定好了的？`tr("Hello")` 在没装翻译的时候返回的是啥，空字符串吗，还是原文？装了多个 translator 的时候，谁的译文排在前头？还有，您拿十六进制工具打开 `.qm` 文件，文件开头的那 16 个字节是什么，是可读的标记串还是二进制？最隐蔽的一个是消息查找，它是哈希表吗，还是别的什么？教材讲到这儿都是含糊的。

这些问题不是散着的，它们落在 Qt i18n 的几根主轴上：`tr` 的来源（`Q_OBJECT` 宏植入的，context 是 moc 在编译期烧进 `staticMetaObject` 的类名），查找链（遍历的是 `translators` 列表，`prepend` 让后装的排前面，全查不到的时候返回的就是原文），QM 文件格式（16 字节的随机 magic、tag-length-value（TLV）分节、没有版本号的字段），查找算法（context 走哈希桶、消息走预排序数组的二分），内部编码（translation 是 UTF-16BE 的，而不是 UTF-8）。这几根轴咱们一根一根地过。

入门篇的 [国际化](../../beginner/01-qtbase/13-i18n-beginner.md) 教了 `tr` 和 `QTranslator` 怎么用，进阶篇的 [国际化进阶：复数规则与动态语言切换](../../advanced/01-qtbase/13-i18n-advanced.md) 补了 `lupdate`/`lrelease` 的流程和复数。本篇咱们要往源码里走，咱们打开 `qtranslator.cpp` 和 `qcoreapplication.cpp`，看 `tr` 是怎么一路走到 `QTranslator::translate` 的，看 `.qm` 文件的二进制长什么样子，看一条消息是怎么被二分搜索找出来的。

边界咱们也划清楚。`tr` 经 `staticMetaObject`（moc 生成）拿 context 的机制，也就是 `className()` 从元对象的 stringdata 表里取类名这一段，咱们只讲编译期烧入类名这个层面。再往深了走，`staticMetaObject` 完整的七字段结构是 [QObject 元对象系统](./01-qobject-meta-system-expert.md) 和 [MOC 编译器原理](./17-moc-compiler-expert.md) 的范畴，本篇就不展开了。`lupdate`/`lrelease` 工具（`.ts` 转 `.qm`、XML 解析、字节码生成）是工具链的主题，本篇讲的是运行时的 `QTranslator` 怎么读 `.qm` 做查找，工具怎么生成 `.qm` 咱们不讲。具体应用里的 `retranslateUi` 实践，咱们只点到 `installTranslator` 发 `LanguageChange` 事件触发这一层，UI 重译的模式就不展开了。

## 环境说明

本篇的源码引用都基于 `qt_src/qt6.9.1`，行号是会随 Qt 版本漂移的，您对照着读的时候，拿函数名去定位是最稳的办法。i18n 涉及的关键文件，咱们列在下面：

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

本篇没有配套的 example，原因和前几篇是一样的，它是纯源码的拆解，您对照着 `qt_src` 翻代码，就是最好的实验。

## 核心概念讲解

咱们动笔讲源码以前，把 `tr` 从源码到翻译结果的完整链路对一遍，下图能帮您看清中间都发生了什么：

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

`tr` 是 `Q_OBJECT` 宏植入的 inline static，它转发给了 `staticMetaObject.tr`，后者把 context 填成 moc 在编译期烧入的类名，交给 `QCoreApplication::translate` 去遍历所有已安装的 `QTranslator`。查到了就返回译文，全部落空了就返回原文。咱们这一篇就顺着它往下拆。

### tr 是 Q_OBJECT 植入的，context 是编译期类名

`tr` 到底是从哪来的？这是咱们要回答的头一个问题。不少人以为 `QObject::tr` 是 `QObject` 类自己定义的方法，不是的，它是 `Q_OBJECT` 宏植进去的（证据在 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtmetamacros.h:84-92`），咱们看：

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

咱们看 `QT_TR_FUNCTIONS` 宏，它展开出来的是一个 inline static 函数，转发给了 `staticMetaObject.tr`。这个宏被 `Q_OBJECT` 塞进了每个 `QObject` 子类的类体里（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtmetamacros.h:133-146`）：

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

所以每个声明了 `Q_OBJECT` 的类，都有自己的 `tr`，它不是继承来的，是宏在类体里展开出来的。这带来一个直接的后果：没声明 `Q_OBJECT` 的类压根就没有 `tr`，您硬要去调，编译期就报错了。

咱们顺着 `tr` 往下走，它转到了 `QMetaObject::tr` 这里，这一步有个要紧的动作是填 context（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qmetaobject.cpp:416-419`）：

```cpp
QString QMetaObject::tr(const char *s, const char *c, int n) const
{
    return QCoreApplication::translate(className(), s, c, n);
}
```

context 不是运行时才拿的类名，它是从 `className()` 来的，而 `className()` 是从 `staticMetaObject` 的 stringdata 表里按索引取出来的字符串，那个字符串是 moc 在编译期写进去的类名。换个直白一点的说法，您写 `MyClass::tr("Hello")`，`translate` 最终收到的 context 就是字符串 `"MyClass"`，这个绑定在编译期就完成了，运行时改不了它。

不是 `QObject` 派生的类（工具类、命名空间）也想用 `tr`，怎么办？咱们就用 `Q_DECLARE_TR_FUNCTIONS`（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.h:257-261`）：

```cpp
#define Q_DECLARE_TR_FUNCTIONS(context) \
public: \
    static inline QString tr(const char *sourceText, const char *disambiguation = nullptr, int n = -1) \
        { return QCoreApplication::translate(#context, sourceText, disambiguation, n); } \
private:
```

宏参数 `context` 被 `#context` 字符串化了，直接传给了 `translate`，它根本不沾 `staticMetaObject` 的边。这是 Qt 给非 `QObject` 类留的口子，咱们用工具类的时候走的就是它。

边角的情况咱们也看一眼：编译时定义了 `QT_NO_TRANSLATION`（把翻译整个禁掉）的时候，`tr` 还在吗？在的，只是退化了（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qobject.h:119-122`）：

```cpp
#if defined(QT_NO_TRANSLATION) || defined(Q_QDOC)
    static QString tr(const char *sourceText, const char * = nullptr, int = -1)
        { return QString::fromUtf8(sourceText); }
#endif
```

此时的 `tr` 直接返回 `QString::fromUtf8(sourceText)`，什么也不查。所以您代码里那些 `tr(...)` 调用，在禁用翻译的构建里不用 `#ifdef` 包裹，照样是能编译、能跑的，只是不翻译而已。

### translate 遍历 translators 列表，全查不到就返原文

本篇的头一个大纠偏点就在这儿了。`tr` 找不到翻译的时候返回的是什么？空字符串吗？笔者当年也是这么以为的，错了。

咱们看 `QCoreApplication::translate` 的实现（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.cpp:2283-2311`）：

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

逻辑咱们分成两段看。头一段遍历的是 `translators` 列表，对每个已安装的 `QTranslator` 调它的 `translate`，命了中（`result.isNull()` 为 false）就 break。第二段是兜底的，遍历完了 `result` 还是 null 的话，走的就是 `result = QString::fromUtf8(sourceText)`，返回的是源文本按 UTF-8 解出来的结果，也就是咱们看到的英文原文。

所以 `tr` 找不到翻译，返回的不是空字符串，其实就是源文本本身。这就是为什么应用就算没装任何翻译，界面也能照常显示英文的，您看，`tr("Hello")` 在没有 translator 的时候返回的就是 `"Hello"`。只有 `sourceText == nullptr` 这个极端的情况才返回空的 `QString`。

末尾那个 `replacePercentN(&result, n)` 是处理复数里 `%n` 占位符的，咱们放到复数一节细讲。

### installTranslator：后装的排前面，不去重

`translators` 列表是怎么维护的，决定了多个 translator 谁的译文能被查到。咱们看 `installTranslator`（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.cpp:2165-2176`）：

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

`prepend`，放到的正是列表头部。结合 translate 一节 `constBegin` 到 `constEnd` 的正向遍历，列表头部是最早被查到的位置，所以后装的 translator 排在前面。官方文档说的 “queries in reverse installation order”（按安装的逆序查询）就是这么来的：您按 A、B、C 的顺序安装，列表就成了 C、B、A，查询的顺序也就是 C、B、A。

这里有个容易栽的地方，笔者专门拎出来讲。`installTranslator` 只做了 nullptr 检查和 instance 检查，是没有 `contains` 去重的，同一个 translator 您 install 两次的话，列表里就会出现两条重复的。查询是幂等的，结果倒是一样的，可这是没有意义的重复，而且 `removeTranslator` 用的 `removeAll` 会一次删掉所有匹配项，您想着卸一次还剩一条，实际是两条一起没了，跟装一次卸一次的预期对不上。

咱们看 `removeTranslator` 的实现（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.cpp:2201-2220`）：

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

`removeAll` 把匹配项全删掉了，重复 install 留下的条目一次就清干净了。返回的 bool 表示的是不是真的移除了东西，没找到的话就不发 `LanguageChange` 事件。您还要注意，`installTranslator` 成功之后和 `removeTranslator` 成功之后都会 `sendEvent` 一个 `LanguageChange`（同步派发的），这是 UI 动态重译的信号源，`QApplication` 把它派发给顶层的 widget，触发它们的 `changeEvent`，您在 `changeEvent` 里重写 `retranslateUi`，语言就能动态切换了。

咱们再看 `translators` 列表本身，它是受读写锁保护的（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication_p.h:158-162`）：

```cpp
#ifndef QT_NO_TRANSLATION
    QTranslatorList translators;
    QReadWriteLock translateMutex;
    static bool isTranslatorInstalled(QTranslator *translator);
#endif
```

`translate` 那边用的是 `QReadLocker`，多线程可以并发地查，`installTranslator` 和 `removeTranslator` 用的是 `QWriteLocker`，独占地写。翻译查询是多线程安全的，这一点您可以放心。

### QM 文件：16 字节随机 magic，没有版本号

咱们打开 `qtranslator.cpp`，看 `.qm` 文件的二进制到底长什么样子。

头一处反直觉的：`.qm` 文件开头那 16 个字节，不少人以为是可读的标记串（比如 `<QTMETADATA>` 之类）。笔者头一回 hexdump 一个 qm 文件的时候也去找过可读头，结果是没找着的，它是随机的二进制（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:51-62`）：

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

咱们看注释里那段 `mcookie` 命令的输出，`3cb86418caef9c95cd211cbf60a1bddd`，就是这 16 字节的来源。`mcookie` 是 Unix 下生成随机 cookie 的工具，Qt 当年拿它随机生成了这串字节，把它固化成了 QM 文件的 magic number。所以这 16 个字节没有任何的语义，纯粹是拿随机数当了文件标识。

第二处反直觉的，咱们接着看：`.qm` 文件没有显式的版本号字段，magic 的后面，直接跟的就是一串 TLV 的结构。section tag 的枚举（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:260`）是这样的：

```cpp
    enum { Contexts = 0x2f, Hashes = 0x42, Messages = 0x69, NumerusRules = 0x88, Dependencies = 0x96, Language = 0xa7 };
```

咱们看 `do_load` 的解析循环长什么样（节选 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:794-836`）：

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

跳过了 16 字节 magic 以后就进了 TLV 循环，1 字节的 tag（`read8`）加 4 字节的大端长度（`read32`）再加数据。认得的 tag 存进对应的字段，不认得的 tag 直接跳过，这是给未来留的余地，往后加了新 section，咱们拿老版本的 Qt 也能把不认识的部分跳过去。整个格式是没有版本号字段的，Qt5 和 Qt6 的 `.qm` 用的是同一套 magic 加 section tag，文件级是兼容的。

您还要留意那个命名错位：`Hashes` 这个 section 在内存里对应的是 `offsetArray`/`offsetLength`，存的是 hash 到消息偏移的二元组。为什么叫 Hashes 又存 offset？这是历史遗留的，您看源码的时候心里要有数。

咱们看，`load` 加载的时候头一件事就是校验 magic（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:539-544`）：

```cpp
        {
            char magicBuffer[MagicLength];
            if (MagicLength != file.read(magicBuffer, MagicLength)
                    || memcmp(magicBuffer, magic, MagicLength))
                return false;
        }
```

您看，长度不足 16 的，或者 magic 对不上的，就都立即返回 false 了，文件根本就不会被加载的。

加载方式上咱们也看一眼，Unix 平台会试着拿 `mmap` 把整个 `.qm` 映射进内存的（节选 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:548-581`）：

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

`PROT_READ` 加 `MAP_PRIVATE` 的只读私有映射，翻译查询是零拷贝的，查找算法直接在映射的内存上跑。mmap 失败了才回退到 `new` 加 `read`。如果您把 `.qm` 放进了 Qt 资源系统而且没有压缩，那连 mmap 都省了，直接复用 `QResource::data()` 的指针（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:511-527`）：

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

资源里没压缩的 QM，咱们就直接拿 `QResource::data()` 的指针来用，同样是零拷贝的。

### 查找算法：context 走哈希桶，消息走二分

`.qm` 加载进了内存以后，`translate` 怎么按 (context, sourceText) 找到译文？这一段咱们看得细一些。

头一步是 context 的查找，咱们看 `do_translate` 在 `Contexts` section 里怎么定位 context（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:957-976`）：

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

`contextArray` 的头 2 字节是桶数 `hTableSize`，咱们拿 `elfHash(context) % hTableSize` 算出落在哪个桶。桶里存的是偏移，咱们顺着偏移跳到桶链上，做读一个长度、比一次字符串的线性扫，比中了就 break，读到长度 0（到了链尾）还没比中就返回空。这就是个哈希加桶内线性探测的结构。有一个要紧的优化：context 在本 QM 里压根不存在的话，这一步就立即 `return QString()` 了，后面的消息查找一步都不用浪费。

消息查找（在 `Hashes` 也就是 `offsetArray` 里）用的是另一套算法，咱们看到的是二分搜索，而不是哈希桶表（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:985-1023` 节选）：

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

本篇的第二个大纠偏点在这儿。笔者翻过的教材，笼统地说了句 “QM 用哈希表查找”，这其实是不准确的。实际的算法是这样的，`offsetArray` 是一个按 hash 升序排好序的数组，每项占了 8 个字节，装的是 hash 的 uint32 加 offset 的 uint32。`while (start <= end)` 那一段就是教科书式的二分搜索，`middle` 取的是中点，比 hash 的大小调整 `start` 和 `end`。比中了 hash 以后，回头扫过所有相同 hash 的项（处理哈希碰撞的），逐项调 `getMessage` 去验证 sourceText、context、comment 是不是真的匹配，哈希只是拿来缩小范围的，它是保证不了唯一的。

hash 是怎么算出来的，您也要看一眼：

```cpp
        quint32 h = 0;
        elfHash_continue(sourceText, h);
        elfHash_continue(comment, h);
        elfHash_finish(h);
```

`h` 是从 0 起步的，咱们拿 `elfHash_continue` 把 sourceText 的字节流喂进去，接着把 comment 的字节流也喂进同一个哈希状态机，末了拿 `elfHash_finish` 收尾。这是累加出来的哈希，不是把 sourceText 和 comment 各算一个再异或的。`elfHash_continue` 的实现在 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:75-101`：

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

4 位的左移、高 4 位的异或回来，咱们熟悉的 Unix ELF 符号表哈希就是它，名字里的 elfHash 就是这么来的。`_continue` 设计成可拼接的样子，就是为了让 (sourceText, comment) 的组合产生单一的哈希键。

外层那个 `for (;;)` 是跑两轮的，头一轮是带着原 comment 查的，查不到的话就把 `comment` 清空了再跑一轮，让带了 disambiguation 的查询，也能命中没填 disambiguation 的译文。这正是 disambiguation 回退的机制，咱们放到 disambiguation 一节接着看。

### 哈希比中以后的精确匹配：getMessage 的变长 TLV

二分找到了匹配的 hash，可光是 hash 还不够的，哈希是会碰撞的。咱们看 `getMessage`，它在 `Messages` section 里解析变长的记录，做的是精确匹配（节选 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:870-935`）：

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
            if (!numerus--) {
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

这是变长的 TLV 结构，每条记录由若干个 `Tag` 开头的字段组成（Tag_Translation、Tag_SourceText、Tag_Context、Tag_Comment、Tag_End），咱们看每个字段，它带一个 4 字节的长度，后面跟的是数据。`match` 函数做的是精确的字节比对，任何一个字段对不上的话，就立即返回空的。有了这一层精确匹配，哈希碰撞就不会误命中了。

接着是本篇的第三个大纠偏点，也是笔者手动解析 qm 的时候栽过的地方，translation 字段的编码。咱们看末尾的几行：

```cpp
    QString str(tn_length / 2, Qt::Uninitialized);
    qFromBigEndian<char16_t>(tn, str.size(), str.data());
    return str;
```

`tn_length / 2`，是拿字节数除以 2 得到的字符数。咱们看 `qFromBigEndian<char16_t>`，它把大端字节流的 16 位单元，转成了 host-endian 的 char16_t，填进了 `QString`。这说明 translation 字段在 `.qm` 里是 UTF-16 大端编码的，而不是 UTF-8。

您往上翻那个长度校验，也印证了这一点：

```cpp
        case Tag_Translation: {
            int len = read32(m);
            if (len & 1)
                return QString();
```

您看 `if (len & 1) return QString()`，长度必须是偶数的，因为 UTF-16 的单位是 2 个字节，奇数的长度就直接判非法了。

所以 `.qm` 的内部编码是不统一的：sourceText、comment、context 这些在 `lupdate` 阶段按 UTF-8 处理，存进 QM 的时候也是 UTF-8 的字节流，靠的是 `strlen` 和 `match` 当裸字节来比。Language section（语言代码的短串）用 `QString::fromUtf8` 解码。唯独 translation 字段是 UTF-16 的大端。您要是手动解析 `.qm`，这个区别您就得记下来。

### 复数 %n：字节码虚拟机

translate 一节末尾留了个 `replacePercentN` 没细讲，它管的就是复数里的 `%n`，咱们在这儿补上。笔者头一回知道 Qt 的复数规则是存成字节码的时候还挺意外的：`.qm` 的 `NumerusRules` section 里是一段字节码，`QTranslator` 用的是一个小型的栈式虚拟机来执行它。

`translate` 拿到了 n 以后，头一件事就是算出复数形式的索引，咱们看（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:982-983`）：

```cpp
    if (n >= 0)
        numerus = numerusHelper(n, numerusRulesArray, numerusRulesLength);
```

执行字节码的活是 `numerusHelper` 干的，咱们看它的实现（节选 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:177-254`）：

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

操作码是按位掩码设计的，咱们看 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator_p.h:20-41` 里的常量定义：

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

咱们把位掩码一位一位地看：低 3 位是操作用的，装的是 EQ、LT、LEQ、BETWEEN。高位是修饰用的，MOD_10 是取个位的，MOD_100 是取百位的，LEAD_1000 是取千位前导的，NOT 是负责取反的。虚拟机执行的时候，`Q_MOD_10` 这类的修饰会对左操作数（n）做变换，再拿低 3 位的操作去跟右操作数比。`Q_AND` 和 `Q_OR` 负责的是组合逻辑表达式，`Q_NEWRULE` 分隔的是多条规则。

这套字节码是能把各语言的复数规则都表达出来的，咱们拿英语看，它的规则就两条，n==1 的时候用 singular（索引 0），其余的时候用 plural（索引 1）。阿拉伯语就复杂多了，“一个”“两个”“少数”“多数”，对应的是好几条 `Q_NEWRULE`。`result` 累加出的是命中的规则索引，回传给了 `getMessage` 当 `numerus` 用，`getMessage` 里的 `if (!numerus--)` 是个倒计数器，逐个地跳过 `Tag_Translation` 字段，数到命中的那个就把它取出来。

最后 `%n` 占位符的替换发生在 `translate` 末尾的 `replacePercentN` 里，咱们看（节选 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qcoreapplication.cpp:2222-2248`）：

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

咱们看它扫的是译文里的 `%n`，替换成 n 的字符串，`%Ln` 用的是本地化格式（带千分位的）。所以 Qt 复数翻译的标准写法是 `tr("%n file(s)", nullptr, count)`，`count` 经 `numerusHelper` 决定的是复数形式，译文里的 `%n` 再被替换成实际的数字。

### disambiguation、依赖链、析构自摘除

咱们说 disambiguation，它就是咱们传的第三参数，Qt5 起用的是新名字，Qt4 的时候叫 comment，查找算法一节咱们喂进哈希的就是它，起的是区分同名 sourceText 的作用，不同的场景靠它分开。它同时进了哈希键和精确匹配（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:986-989,917-923`）：

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

所以 `tr("Open", "File menu")` 和 `tr("Open", "Disk operation")` 会得到不同的 hash、落进不同的桶、被正确地区分开。您再注意精确匹配里 `if (*m && ...)` 那个条件，QM 里存的 comment 是空串（首字节 0）的话，就不强制匹配了，这其实等于允许没填 disambiguation 的译文，去匹配查询时带了 disambiguation 的请求。它跟查找算法一节外层 `for (;;)` 的两轮回退是配套的机制。

咱们还能让 `.qm` 声明对别的 `.qm` 的依赖（`Dependencies` section，`lrelease` 的时候指定），依赖命中的转发在 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:1029-1035`：

```cpp
searchDependencies:
    for (const auto &translator : subTranslators) {
        QString tn = translator->translate(context, sourceText, comment, n);
        if (!tn.isNull())
            return tn;
    }
    return QString();
```

咱们再看依赖的部分。主 translator 没查到的时候，就向它加载进来的子 translator（`subTranslators`）转发查询。加载的时候会把所有的依赖递归 `load` 一遍，任何一个失败了就全部清理（fail-fast 的路子）。这个机制常用的场景是“基础语言包加地区变体”。

最后咱们看 `QTranslator` 的析构，这是一处容易栽的地方（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtranslator.cpp:406-412`）：

```cpp
QTranslator::~QTranslator()
{
    if (QCoreApplication::instance())
        QCoreApplication::removeTranslator(this);
    Q_D(QTranslator);
    d->clear();
}
```

咱们看到析构的时候，它主动调了 `removeTranslator`，把自己从 `QCoreApplication` 里摘了出去。为什么要这样？`QCoreApplication` 是不拥有 `QTranslator` 所有权的（文档里明确写了 `does not take ownership`），可 translator 装进去了以后，`QCoreApplication` 手里拿的是它的裸指针，translator 要是赶在 `QCoreApplication` 前头析构了，那个指针就悬空了。所以 `~QTranslator` 自己摘除自己来防悬挂（前提是 `QCoreApplication` 还活着的）。这也是 translator 通常建议做成栈对象或者挂到对象树上的原因，靠生命周期自动地管摘除。

## 风险与故障预防

本篇讲的风险都是从源码里能直接对上号的，笔者自己也真栽过，这里咱们把后果和解法一个个交代清楚。

头一类的风险是拿 `tr` 的返回值判断翻译生没生效。您写 `if (tr("Hello").isEmpty())`，想判断的是“翻译是否生效”，这个分支是永远进不去的，因为 `tr` 找不到的时候返回的是 `"Hello"` 本身，而不是空串。整个应用看着翻译功能是正常的（界面显示英文），其实您的 translator 根本就没加载成功，您还以为它加载上了。根因是 translate 一节讲的兜底逻辑，`if (result.isNull()) result = QString::fromUtf8(sourceText);`，只有 `sourceText == nullptr` 才返回空的 `QString`，正常的 miss 都返回原文。正确的做法是查 `QTranslator::isEmpty()`（加载完以后里头有没有真内容），或者干脆看界面的文案变没变，别拿 `tr(...).isEmpty()` 当您的判据。

第二类是重复的 install。您在某个槽函数里每次响应语言切换都调 `installTranslator(myTranslator)`，想着的是“重新装一下刷新”，实际的列表里，这个 translator 越积越多的（每次 prepend 一条）。查询是幂等的，结果是不变的，可 `translators` 列表是越拉越长的，每次 `translate` 都要多白查几遍的。更糟的是您以为，“卸载一次”就能清掉的，结果 `removeAll` 是一次删光的，行为跟您的预期对不上。根因是 installTranslator 一节讲的，它只做 prepend 不做去重的。正确的做法是 translator 全局只 install 一次，同一个对象您别反复 install。要切换语言的话，把旧的 `removeTranslator` 掉，拿 `load` 把新的 QM 装进同一个 translator 对象，靠的是 `LanguageChange` 事件触发刷新。您也可以走另一条路，remove 掉旧的，再 install 新的。

第三类是多个 translator 的次序搞反了。您装了两个 translator，A 是通用的翻译，B 是行业术语的补丁，指望的是 A 兜底、B 覆盖特定词条，结果发现 B 的覆盖没生效，查到的是 A 的译文。也有反过来的情况，您不知道谁覆盖谁，翻译的结果看着时好时坏的。根因是 prepend 加了正向遍历，列表的头部放的是最新装的，它是最早被查的，您按 A、B 的顺序安装，查询的顺序就是 B、A，B 比中了就返回，轮不到 A 的。正确的做法是认清“后装的排前面”的次序，想让补丁 translator 覆盖基础的，补丁就得是后 install 的那个。要调整次序的话，把两个都 remove 干净了，按您期望的顺序重新 install，末了装的是最靠前的。

第四类是没有 `Q_OBJECT` 的类没有 `tr`。您在一个没继承 `QObject` 的工具类里写 `tr("Hello")`，编译直接就报错了，说的是 “no member named ‘tr’”。或者类继承了 `QObject` 但忘了拍 `Q_OBJECT` 宏，调 `tr` 一样是编译失败的。根因是 tr 植入一节讲的，`tr` 是 `Q_OBJECT` 植入的，没有 `Q_OBJECT` 的话就没有 `tr`。正确的做法是 `QObject` 派生类把 `Q_OBJECT` 拍上，非 `QObject` 的类（工具类、命名空间）用 `Q_DECLARE_TR_FUNCTIONS(ClassName)` 宏注入 `tr`，context 是宏参数字符串化得到的，展开的样子咱们在 `Q_DECLARE_TR_FUNCTIONS` 那里看过。

## 练习项目

本篇跟前几篇是一样的，走的是纯源码拆解的路子，练习也就走验证的路子，四道题给的是方向和验证的手段，完整的工程留给您自己搭。

头一道题验的是“`tr` 查不到返回原文”，您亲手过一遍。您写一个最小的 `QObject` 子类，把 `Q_OBJECT` 拍上了，在里头调了 `tr("Hello")`，什么都不装的情况下把返回值打出来，您看到的返回值就是 `"Hello"` 本身的，而不是空串。跟着您再拿 `tr(...).isEmpty()` 判一下，它是恒为 false 的，这正是风险一节讲的误判来源。对照的办法是 `QTranslator::isEmpty()`，加载成功了它是 false，加载失败了（文件不存在、magic 对不上）它就是 true，两条路一比的话，判据该用哪个就清楚了。

第二道题演的是重复 install 和一次删光。您对同一个 `QTranslator` 对象连调了两次 `installTranslator`，然后只 `removeTranslator` 了一次，再查 `tr` 的时候，译文已经没有了。只卸了一次却全没了，这正是 `removeAll` 把两条一起删掉的表现，跟您“装一次卸一次”的直觉正好对不上，源码的对照就在 installTranslator 一节。

第三道题验的是次序。您准备两个 `.qm`，给同一个 key 配上了不同的译文，咱们把它们做成两个 translator，按 A、B 的顺序 install，`tr` 查到的是 B 的译文。把两个都 remove 掉了，按 B、A 的顺序重新装一遍，查到的就换成了 A 的译文。后装的那个是生效的，您亲手换一遍就有体感了。

第四道题是回源码和二进制里做核对的。您拿十六进制工具打开任意一个 `.qm`，文件开头的 16 个字节应当是 `3c b8 64 18 ca ef 9c 95 cd 21 1c bf 60 a1 bd dd`，正是 magic 一节列出的那串 mcookie 输出。行号核对的任务也都在前文里，`qtranslator.cpp:260` 的 section tag 枚举、`qtranslator.cpp:870-935` 末尾 `qFromBigEndian<char16_t>` 那两行（translation 是 UTF-16 大端的证据）、`qcoreapplication.cpp:2283-2311` 的兜底返回，咱们一处一处地对下来，都对上了，本篇的断言也就落了地。

## 官方文档参考链接

[QTranslator 文档](https://doc.qt.io/qt-6/qtranslator.html)——`load`/`translate`/`isEmpty` 的查询入口，背后的实现咱们在查找算法一节拆过。

[QCoreApplication::translate 文档](https://doc.qt.io/qt-6/qcoreapplication.html#translate)——参数的说明都在，查不到返原文的行为也在这一页，咱们跟 translate 一节的源码对着看。

[QCoreApplication::installTranslator 文档](https://doc.qt.io/qt-6/qcoreapplication.html#installTranslator)——查询的逆序，这一页的官方说明里写着，咱们在源码里看到的 `prepend` 就是它的实现。

[Qt Linguist Manual](https://doc.qt.io/qt-6/linguist-manual.html)——工具链的官方手册在这里，`lupdate`/`lrelease` 的用法咱们查它，运行时怎么读 `.qm` 的，看本篇就够了。

[Writing Source Code for Translation](https://doc.qt.io/qt-6/i18n-source-translation.html)——用法的规范在这一页，`tr`、`Q_DECLARE_TR_FUNCTIONS`、`qtTrId` 咱们写源码的时候照着它来。

[Plural Form Support](https://doc.qt.io/qt-6/i18n-source-translation.html#plural-form)——`%n` 的官方说明在这里，复数规则跟字节码虚拟机一节是对着看的，咱们两处一起翻。

---

Qt 的国际化，走的是源码无感加运行时按需查找的路子。您在源码里写 `tr`，编译期 moc 把类名烧进了 `staticMetaObject` 当 context，运行时的 `QCoreApplication` 遍历 translator 列表，每个 `QTranslator` 在 `.qm` 的二进制结构上做哈希加二分的查找。源码和翻译资源就这样解了耦，切换语言是不用重编程序的。不过反直觉的细节也都藏在里头：`tr` 查不到的时候返回的是原文，后装的是排在前面的，install 是不去重的，QM 用的是随机 magic，消息走的是二分不是哈希表，translation 是 UTF-16 的，而 sourceText 是 UTF-8。这些细节平时是用不上的，可您一旦要做翻译调试、性能优化，或者要手动解析 QM 的话，它们就是必须懂的底层。这一篇读完了，翻译的行为要是再诡异起来，您该知道往哪儿查了。

您要是想把本篇的行号证据拿来一一核对，它们已经按源码机制归类收在 [code-index · QTranslator 与 QM 文件格式源码索引](../code-index/qtbase/qtranslator.md) 底下了，您带着行号直接去 `qt_src/qt6.9.1` 翻原文就行。
