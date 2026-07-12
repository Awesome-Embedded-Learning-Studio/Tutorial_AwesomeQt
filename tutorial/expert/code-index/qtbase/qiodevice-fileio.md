---
title: QIODevice 抽象基类与 QFile/QSaveFile 源码索引
description: QIODevice 双继承与 OpenMode 标志、随机/顺序双模式与 seek 守卫、readData/writeData 纯虚契约、QRingBuffer 缓冲与两路径 read、readLine 与 Text 模式、信号发射方（readyRead/bytesWritten 子类 emit、aboutToClose 自 emit）、QFile→QFileDevice→FileEngine 三层与平台 open 映射、QSaveFile 原子写四件套。
---

# QIODevice 抽象基类与 QFile/QSaveFile 源码索引

> 本索引收录 Qt 6.9.1 源码中 QIODevice 抽象基类 + QFile/QFileDevice/QSaveFile 的已验证证据。QIODevice 是 Qt 所有 IO 类（QFile/QBuffer/QProcess/QAbstractSocket/QTcpSocket）的共同基类。

## QIODevice 双继承与 OpenMode 标志

源码文件：`qtbase/src/corelib/io/qiodevice.h` / `qiodevicebase.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 双继承 QObject + QIODeviceBase | qiodevice.h:30-37 | `class QIODevice : public QObject, public QIODeviceBase`（带 `#ifndef QT_NO_QOBJECT` 分支） | QIODeviceBase 是 Qt6 拆出的非模板纯数据基类（析构 protected），只放 OpenMode，避免循环依赖。QObject 可用 QT_NO_QOBJECT 编译关掉。 |
| OpenMode 10 个位标志全在 QIODeviceBase | qiodevicebase.h:16-28 | `enum OpenModeFlag { NotOpen=0x0000, ReadOnly=0x0001, WriteOnly=0x0002, ReadWrite=ReadOnly\|WriteOnly, Append=0x0004, Truncate=0x0008, Text=0x0010, Unbuffered=0x0020, NewOnly=0x0040, ExistingOnly=0x0080 }; Q_DECLARE_FLAGS(OpenMode, OpenModeFlag)` | ReadWrite 是位运算组合（值 0x3）。Text/Unbuffered/NewOnly/ExistingOnly 是修饰位。集中在 QIODeviceBase 印证拆基类动机。 |

## 双模式与 seek 守卫

源码文件：`qtbase/src/corelib/io/qiodevice.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| isSequential 默认 false（随机访问） | :493-496 | `bool QIODevice::isSequential() const { return false; }` | QFileDevice 覆盖它委托 fileEngine->isSequential()——普通文件返 false，管道/串口等返 true。正文别泛化「QFileDevice 永远 false」。 |
| seek 三道守卫 + devicePos/seekBuffer | :857-886 | `if (d->isSequential()) { checkWarnMessage(...); return false; } if (d->openMode == NotOpen) {...} if (pos < 0) {...} ... d->devicePos = pos; d->seekBuffer(pos); return true;` | sequential 设备 seek 直接 false（带警告）；通过后更新 devicePos 并 seekBuffer（清/截 buffer）。 |

## readData/writeData 纯虚契约（子类核心）

源码文件：`qtbase/src/corelib/io/qiodevice.h` / `qiodevice.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| readData 是纯虚 = 0 | qiodevice.h:132 | `virtual qint64 readData(char *data, qint64 maxlen) = 0;` | 不是「默认返回 -1」（那是 Qt4/5 旧印象）。QIODevice 是抽象类，qiodevice.cpp 仅 `\fn` doc 无函数体；子类 QFileDevice::readData（qfiledevice.cpp:462）才是真身。 |
| writeData 也是纯虚 = 0 | qiodevice.h:135 | `virtual qint64 writeData(const char *data, qint64 len) = 0;` | 同 readData。write() 主路径调 writeData，Win 平台 `#ifdef Q_OS_WIN` 的 Text 模式按块扫描 `\n` 分次 writeData 做 `\n→\r\n` 转换（仅 Win）。 |
| read 公共薄壳 + getChar 快捷 | qiodevice.cpp:996-1039 | `CHECK_READABLE(read, qint64(-1)); if (maxSize == 1 && ...) { while ((chint = d->buffer.getChar()) != -1) {...} } ... const qint64 readBytes = d->read(data, maxSize);` | maxSize==1 的 getChar 快捷路径仍调 readData(data,0) 预填缓冲（非绕过）；真逻辑在 QIODevicePrivate::read。Text 模式吃掉 `\r`。 |

## QRingBuffer 缓冲与两路径 read

源码文件：`qtbase/src/corelib/io/qiodevice_p.h` / `qiodevice.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 缓冲结构：QRingBufferRef（引用）+ QVarLengthArray（拥有）+ chunkSize 16384 | qiodevice_p.h:105-116, :30-31 | `QRingBufferRef buffer; QRingBufferRef writeBuffer; int readBufferChunkSize = QIODEVICE_BUFFERSIZE; QVarLengthArray<QRingBuffer, 2> readBuffers; QVarLengthArray<QRingBuffer, 1> writeBuffers;`（`#define QIODEVICE_BUFFERSIZE 16384`） | QRingBufferRef 持裸指针 m_buf（引用非拥有），真实所有权在 readBuffers/writeBuffers。setCurrentReadChannel 把 `&readBuffers[channel]` 挂到 buffer.m_buf。 |
| read 两路径：大块直读 vs 填充缓冲 | qiodevice.cpp:1048, :1082-1116 | `const bool buffered = (readBufferChunkSize != 0 && (openMode & Unbuffered) == 0); if ((!buffered \|\| maxSize >= readBufferChunkSize) && !keepDataInBuffer) readFromDevice = q->readData(data, maxSize); else readFromDevice = q->readData(buffer.reserve(bytesToBuffer), bytesToBuffer);` | maxSize>=16KB 或 Unbuffered → 直读用户缓冲绕过 ring；小读 → 先填 ring，下次 read 命中 buffer。 |

## readLine 与 Text 模式

源码文件：`qtbase/src/corelib/io/qiodevice.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| readLine 先 peek buffer 再回退 readLineData；Text 出口做 \r\n→\n | :1346-1410, :1622-1644 | `if (!buffer.isEmpty()) readSoFar = buffer.readLine(...); if (data[readSoFar-1]=='\n') { if (openMode & Text) { if (data[readSoFar-2]=='\r') { --readSoFar; data[readSoFar-1]='\n'; } } return readSoFar; }` / readLineData `while (...) { *data++ = c; if (c == '\n') break; }` | buffer 里有 `\n` 就 peek 出来避免 readData；不够回退 readLineData（默认逐字节 read，性能差，QFileDevice 可 override 优化）。Text 转换在 readLine 出口。 |

## 信号契约（谁发射）

源码文件：`qtbase/src/corelib/io/qiodevice.h` / `qiodevice.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| readyRead/bytesWritten 由子类 emit，QIODevice 自己不 emit | :118, :120 | `void readyRead(); void bytesWritten(qint64 bytes);`（Q_SIGNALS 块） | 信号声明在 QIODevice，但 grep qiodevice.cpp 无 emit readyRead/bytesWritten（仅 doc 提及）。由 QAbstractSocket 等子类在 OS notify 回调里发射。异步 IO 驱动契约。 |
| aboutToClose 是 QIODevice 唯一自 emit 的信号 | :122 + qiodevice.cpp:787-807 | `void aboutToClose();` / close() 里 `... emit aboutToClose(); d->openMode = NotOpen;` | 在 close() 改 openMode 之前发射，让订阅者能 flush 等。writeBuffers 不清空（为 socket delayed close 留尾巴）。 |

## QFile→QFileDevice→FileEngine 三层与平台 open

源码文件：`qtbase/src/corelib/io/qfile.h` / `qfiledevice.h` / `qfiledevice_p.h` / `qfile.cpp` / `qfsfileengine_unix.cpp` / `qfsfileengine_win.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 三层继承链 | qfile.h:92, qfiledevice.h:31 | `class QFile : public QFileDevice` / `class QFileDevice : public QIODevice` | QFileDevice 是 Qt5.0 拆出的中间层（文件句柄封装），让 QTemporaryFile/QSaveFile 共享。 |
| QFileDevicePrivate 持 FileEngine 不持 fd | qfiledevice_p.h:58 | `mutable std::unique_ptr<QAbstractFileEngine> fileEngine;` | 全文无 FILE* fh 或 int fd 字段。真 fd 在 QFSFileEnginePrivate（unix）、fileHandle（win）。推翻「QFile 直接封装 fd」误解。 |
| QFile::open 强制 Unbuffered + engine 调平台 native | qfile.cpp:931-951, qfsfileengine_unix.cpp:35-60, :88-96 | `if (d->engine()->open(mode \| QIODevice::Unbuffered)) {...}` / `Q_ASSERT_X(openMode & Unbuffered, ...); fd = QT_OPEN(...);` / Win `fileHandle = CreateFile(...)` | 「QIODevice 做 buffering，engine 只做裸 IO」——QFSFileEngine 有断言不再支持 buffered。Unix 走 QT_OPEN（POSIX open，非 stdio fopen），Win 走 CreateFile。 |

## QSaveFile 原子写

源码文件：`qtbase/src/corelib/io/qsavefile.h` / `qsavefile.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| close 私有 + qFatal 硬阻止 | qsavefile.h:53-54, qsavefile.cpp:272-274 | `private: void close() override;` / `void QSaveFile::close() { qFatal("QSaveFile::close called"); }` | 强制必须走 commit，不能误调普通 close。 |
| commit 四件套：flush→syncToDisk→renameOverwrite | qsavefile.cpp:289-330 | `QFileDevice::close(); fe->syncToDisk(); if (d->useTemporaryFile) { if (d->error != NoError) { fe->remove(); return false; } if (!fe->renameOverwrite(d->finalFileName)) { fe->remove(); return false; } }` | flush + fsync + 无错原子 rename 替换 / 有错 remove 丢弃。 |
| writeData 捕获错误存 writeError | qsavefile.cpp:362-373 | `if (d->writeError != NoError) return -1; const qint64 ret = QFileDevice::writeData(data, len); if (d->error != NoError) d->writeError = d->error;` | 一次写失败后后续 write 直接返 -1，commit 时据 writeError 决定丢弃。 |
