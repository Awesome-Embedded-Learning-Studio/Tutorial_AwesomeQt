---
title: "5.11 NFC 进阶：NDEF 记录类型详解与写入标签"
description: "入门篇我们把 QNearFieldManager 的基本 NFC 标签检测跑通了——检测到标签靠近时发出信号。但 NFC 的真正价值不在于「检测到标签」，而在于读写标签上的 NDEF（NFC Data Exchange Format）消息。NDEF 是 NFC 论坛定义的标准数据格式，一条 NDEF 消息由一个或多个 NDEF 记录组成，每条记录携带类型化的数据——文本、URI、Smart Poster 等。"
---

# 现代Qt开发教程（进阶篇）5.11——NFC 进阶：NDEF 记录类型详解与写入标签

## 1. 前言

入门篇我们把 QNearFieldManager 的基本 NFC 标签检测跑通了——检测到标签靠近时发出信号。但 NFC 的真正价值不在于「检测到标签」，而在于读写标签上的 NDEF（NFC Data Exchange Format）消息。NDEF 是 NFC 论坛定义的标准数据格式，一条 NDEF 消息由一个或多个 NDEF 记录组成，每条记录携带类型化的数据——文本、URI、Smart Poster 等。

这篇我们把 NDEF 记录的解析、构造、以及写入标签的完整流程拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::Nfc 模块。NFC 硬件方面，需要支持 NDEF 的 NFC 标签（如 NTAG213/215/216）和 NFC 读卡器。Android 设备通常自带 NFC 硬件，Linux 桌面需要 USB NFC 读卡器（如 ACR122U）。Qt NFC 在 Windows 桌面上的支持有限。

## 3. 核心概念讲解

### 3.1 NDEF 记录类型

NDEF 记录的核心是类型（Type）字段，它告诉读取方这条记录携带的是什么数据。Qt 提供了几个预定义的 NDEF 记录子类来处理常见类型。

`QNdefNfcTextRecord` 用于存储文本。它包含文本内容和一个语言标签（如 "en"、"zh"）。一个 NDEF 消息中可以有多条不同语言的文本记录，读取方根据语言偏好选择显示哪一条。

`QNdefNfcUriRecord` 用于存储 URI（网址、电话号码等）。URI 在记录中被压缩存储——常见的前缀（`http://`、`https://`、`tel:`）用一个字节代码表示，节省标签空间。

`QNdefNfcSmartUriRecord`（Smart Poster）是更复杂的记录类型，它把 URI、标题文本和可选的动作（如「打开浏览器」、「发送短信」）打包在一起。

```cpp
// 解析标签上的 NDEF 消息
void onTargetDetected(const QNdefMessage &message)
{
    for (const QNdefRecord &record : message) {
        if (record.isFormatType(QNdefNfcTextRecord::uri())) {
            QNdefNfcTextRecord textRecord(record);
            qDebug() << "Text:" << textRecord.text()
                     << "Locale:" << textRecord.locale();
        } else if (record.isFormatType(QNdefNfcUriRecord::uri())) {
            QNdefNfcUriRecord uriRecord(record);
            qDebug() << "URI:" << uriRecord.uri().toString();
        }
    }
}
```

### 3.2 构造并写入 NDEF 消息

写入标签需要先构造 NDEF 消息，然后通过 QNearFieldTarget 的 `writeNdefMessages()` 方法写入。

```cpp
void writeTextToTag(QNearFieldTarget *target,
                     const QString &text)
{
    // 构造文本记录
    QNdefNfcTextRecord textRecord;
    textRecord.setText(text);
    textRecord.setLocale("zh");
    textRecord.setEncoding(QNdefNfcTextRecord::Utf8);

    QNdefMessage message;
    message.append(textRecord);

    // 写入标签
    target->writeNdefMessages({message});
}
```

写入是异步操作——`writeNdefMessages()` 返回一个 request ID，写入完成后 `QNearFieldTarget` 会发出 `ndefMessageWritten` 或 `error` 信号。注意写入时标签必须保持在读卡器的感应范围内——如果在写入过程中移开标签，数据可能被截断或损坏。

## 4. 踩坑预防

NFC 标签的存储容量非常有限。NTAG213 只有 144 字节可用空间，NTAG215 有 504 字节，NTAG216 有 888 字节。一条 NDEF 消息加上记录头开销后可能比原始数据大 10-20%。如果写入失败，检查消息大小是否超过标签容量。

## 5. 练习项目

练习项目：NFC 名片写入器。将联系人的姓名、电话、邮箱打包成多条 NDEF 记录（文本记录 + URI 记录），写入 NFC 标签。读取时解析并显示完整的联系人信息。

## 6. 官方文档参考链接

[Qt 文档 · Qt NFC](https://doc.qt.io/qt-6/qtnfc-index.html) -- NFC 模块总览

[Qt 文档 · QNdefMessage](https://doc.qt.io/qt-6/qndefmessage.html) -- NDEF 消息容器

[Qt 文档 · QNdefNfcTextRecord](https://doc.qt.io/qt-6/qndefnfctextrecord.html) -- NDEF 文本记录

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。NDEF 记录的解析、构造和写入——搞定了这些，你的 NFC 应用就能读写标准 NDEF 标签了。
