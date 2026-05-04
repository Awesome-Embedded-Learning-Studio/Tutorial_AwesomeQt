# 现代Qt开发教程（新手篇）5.11——QtNFC 近场通信基础

## 1. 前言：NFC 的尴尬定位

NFC（Near Field Communication，近场通信）在日常生活中出现的频率比很多人意识到的要高——门禁卡、公交卡、银行卡闪付、Android Beam（虽然已经废弃了）、NFC 标签贴纸，全都用的这套协议。它的核心特征是通信距离极短（通常 10 厘米以内，实际使用基本贴着），工作频率 13.56 MHz，数据传输速率在 106-424 kbps 之间。这个"贴着才能用"的特性既是限制也是优势——天然防窃听，不需要配对，碰一下就完事。

NFC 开发在 Qt 生态里一直是个比较冷门的方向。QtNfc 模块提供的 API 其实设计得不错，但平台支持的限制非常明显：它主要在 Android 上工作得最好，iOS 上由于 Apple 的沙盒限制只能使用 NDEF 标签读取（而且要在 App 里声明 NFC 权限），Linux 桌面基本不支持，Windows 也不太行。所以如果你打算做 NFC 开发，目标平台基本就锁定 Android。这篇我们会在概念层面把 NFC 标签类型、NDEF 格式、读写流程全部讲清楚，示例代码以 Android 为目标平台编写，桌面环境可以编译但不一定有实际的 NFC 硬件交互。

NFC 的协议栈比蓝牙简单得多，但它有一套独特的标签类型体系和 NDEF 数据格式，这些概念必须搞清楚才能写出正确的读写代码。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 QtNfc 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Nfc Widgets)
```

QtNfc 在 Qt 6 中是一个附加模块。在 Qt Installer 里需要确保勾选了 QtNfc 组件。某些发行版的包管理器中可能叫 `qt6-nfc`。从源码编译时 QtNfc 在 `qtnfc` 仓库中。

目标平台主要是 Android（需要 NFC 硬件支持，大部分 Android 手机都有），iOS 上有限支持。Linux/Windows 桌面环境通常不具备 NFC 硬件，QtNfc 的后端实现也不完善。Android 上需要在 AndroidManifest.xml 中声明 NFC 权限：

```xml
<uses-permission android:name="android.permission.NFC" />
<uses-feature android:name="android.hardware.nfc" android:required="false" />
```

编译工具链方面，Android 开发需要 Android NDK 和 Qt for Android 工具链。桌面端编译测试 MSVC 2019+、GCC 11+ 均可，C++17 标准，CMake 3.26+ 构建系统。

## 3. 核心概念讲解

### 3.1 NFC 标签类型——Type 1 到 Type 5 的来龙去脉

NFC 标签不是一个统一标准，而是多种标签技术的统称。NFC Forum 定义了五种标签类型（Type 1 到 Type 5），它们对应不同的底层芯片标准和通信协议。

Type 1 基于 Innovision Jewel/Broadcom Topaz 芯片，存储容量最小（通常 96 字节到 512 字节），成本最低，读写速度也最慢。典型应用是廉价的 NFC 贴纸，只能存一个 URL 或少量文本。

Type 2 基于 NXP MIFARE Ultralight 芯片，存储容量 48 字节到 888 字节，是消费级 NFC 标签中最常见的类型。你在淘宝上买到的大部分 NFC 贴纸（白色圆形小标签那种）都是 Type 2。它支持 NDEF 格式，价格便宜，读取可靠，是入门 NFC 开发最常用的标签类型。

Type 3 基于 Sony FeliCa 芯片，主要用于日本市场。交通卡（Suica、Pasmo）使用这种标签。数据传输速度较快但成本较高，在中国大陆和欧美市场不太常见。

Type 4 基于 NXP MIFARE DESFire 系列芯片，存储容量最大（可达 32KB 以上），支持高级安全特性（加密、访问控制），成本也最高。门禁卡、银行卡的非接部分通常使用 Type 4 标签。它分 A 类和 B 类两种变体，区别在于通信协议的细节。

Type 5 基于 NXP ICODE 和 TI Tag-it 芯片，支持 NFC-V（ISO 15693）协议，读写距离比其他类型稍远（可达 1.5 米），主要用于资产追踪和库存管理。

QtNfc 的 API 对这些标签类型做了抽象，开发者通常不需要直接处理底层协议差异。但了解标签类型对排查问题很重要——比如 Type 2 标签的存储空间很小，写入大的 NDEF 消息可能会失败；Type 4 标签可能需要认证才能读写。

### 3.2 NDEF——NFC 的通用数据格式

NDEF（NFC Data Exchange Format）是 NFC Forum 定义的通用数据封装格式，用于在 NFC 标签上存储结构化数据。不管底层是 Type 2 还是 Type 4 标签，NDEF 都可以在上面工作——它是标签类型之上的统一数据层。

NDEF 的结构是"消息-记录"的层次模型。一个 NDEF 消息（QNdefMessage）包含一条或多条 NDEF 记录（QNdefRecord）。每条记录有一个类型（Type）、一个载荷（Payload）和一些标志位（如是否为第一条/最后一条记录、载荷长度等）。QNdefMessage 本质上是 QNdefRecord 的 QList。

QtNfc 提供了一些预定义的 NDEF 记录类型来简化常见数据的操作：

```cpp
#include <QNdefMessage>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>

// 创建一个包含文本记录的 NDEF 消息
QNdefMessage message;

QNdefNfcTextRecord textRecord;
textRecord.setText("Hello from Qt NFC!");
textRecord.setLocale("en");
textRecord.setEncoding(QNdefNfcTextRecord::Utf8);
message.append(textRecord);

// 创建一个包含 URI 记录的 NDEF 消息
QNdefNfcUriRecord uriRecord;
uriRecord.setUri(QUrl("https://doc.qt.io"));
message.append(uriRecord);
```

QNdefNfcTextRecord 用于存储文本数据，支持多语言（通过 locale 字段）和两种编码（UTF-8 和 UTF-16）。QNdefNfcUriRecord 用于存储 URI，NFC Forum 为 URI 定义了缩写前缀——比如 0x00 表示无前缀，0x01 表示 "http://www."，0x04 表示 "https://"，这样可以节省标签存储空间。Qt 自动处理这些前缀。

写入 NDEF 消息到标签：

```cpp
// 将 NDEF 消息序列化为字节数组
QByteArray data = message.toByteArray();
```

读取标签上的 NDEF 消息：

```cpp
// 从字节数组解析 NDEF 消息
QNdefMessage message = QNdefMessage::fromByteArray(rawData);

for (const QNdefRecord &record : message) {
    if (record.isFormatType<QNdefNfcTextRecord>()) {
        QNdefNfcTextRecord textRecord(record);
        qDebug() << "文本记录:" << textRecord.text();
    } else if (record.isFormatType<QNdefNfcUriRecord>()) {
        QNdefNfcUriRecord uriRecord(record);
        qDebug() << "URI 记录:" << uriRecord.uri().toString();
    }
}
```

isFormatType<T>() 是 Qt 提供的模板方法，用于检查一条 NDEF 记录是否是特定的类型。这是类型安全的做法，比手动检查 record type 字段可靠得多。

### 3.3 QNearFieldManager——NFC 标签检测的核心

QNearFieldManager 是 QtNfc 的入口类，负责检测 NFC 标签的靠近和移开。它的使用方式和 QBluetoothDeviceDiscoveryAgent 类似——设置目标模式、注册信号、启动检测：

```cpp
#include <QNearFieldManager>
#include <QNearFieldTarget>

auto *nfcManager = new QNearFieldManager(this);

// 检查设备是否支持 NFC
if (!nfcManager->isAvailable()) {
    qWarning() << "本设备不支持 NFC";
    return;
}

// 标签靠近时触发
connect(nfcManager, &QNearFieldManager::targetDetected,
        this, [](QNearFieldTarget *target) {
    qDebug() << "检测到 NFC 标签:"
             << "类型:" << target->type()
             << "UID:" << target->uid().toHex();

    // 读取标签上的 NDEF 消息
    // ...
});

// 标签移开时触发
connect(nfcManager, &QNearFieldManager::targetLost,
        this, [](QNearFieldTarget *target) {
    qDebug() << "NFC 标签已移开";
    target->deleteLater();
});

// 设置检测模式为 NDEF 标签
nfcManager->setTargetAccessModes(
    QNearFieldManager::NdefRead);

// 开始检测
nfcManager->startTargetDetection();
```

QNearFieldManager 的 targetAccessModes 决定了检测什么类型的标签和以什么模式操作。NdefRead 表示检测支持 NDEF 读取的标签，NdefWrite 表示检测支持 NDEF 写入的标签，你可以组合使用。还有一个 TagTypeSpecific 模式用于需要直接操作底层标签协议的场景。

targetDetected 信号携带一个 QNearFieldTarget 指针，它代表被检测到的 NFC 标签。你可以通过这个对象读取标签数据、写入 NDEF 消息、获取标签类型和 UID 等信息。

### 3.4 QNearFieldTarget——读写标签数据

QNearFieldTarget 封装了与单个 NFC 标签的所有交互。读取和写入操作都是异步的——调用后返回一个 request id，结果通过信号通知。

读取 NDEF 消息：

```cpp
connect(nfcManager, &QNearFieldManager::targetDetected,
        this, [this](QNearFieldTarget *target) {

    // 请求读取 NDEF 消息
    QNearFieldTarget::RequestId id =
        target->readNdefMessages();

    if (!id.isValid()) {
        qWarning() << "读取请求失败";
        return;
    }

    // 监听读取结果
    connect(target, &QNearFieldTarget::ndefMessageRead,
            this, [](const QNdefMessage &message) {
        qDebug() << "读取到 NDEF 消息，包含"
                 << message.size() << "条记录";

        for (const QNdefRecord &record : message) {
            if (record.isFormatType<QNdefNfcTextRecord>()) {
                QNdefNfcTextRecord text(record);
                qDebug() << "文本:" << text.text();
            } else if (record.isFormatType<QNdefNfcUriRecord>()) {
                QNdefNfcUriRecord uri(record);
                qDebug() << "URI:" << uri.uri().toString();
            } else {
                qDebug() << "未知记录类型:"
                         << record.type().toHex();
            }
        }
    });

    // 监听错误
    connect(target, &QNearFieldTarget::error,
            this, [](QNearFieldTarget::Error error,
                     const QNearFieldTarget::RequestId &id) {
        qWarning() << "NFC 操作错误:"
                   << static_cast<int>(error);
    });
});
```

写入 NDEF 消息：

```cpp
void writeToTarget(QNearFieldTarget *target)
{
    // 构造要写入的 NDEF 消息
    QNdefMessage message;

    QNdefNfcUriRecord uriRecord;
    uriRecord.setUri(QUrl("https://www.qt.io"));
    message.append(uriRecord);

    QNdefNfcTextRecord textRecord;
    textRecord.setText("Written by Qt NFC");
    textRecord.setLocale("en");
    message.append(textRecord);

    // 写入
    QNearFieldTarget::RequestId id =
        target->writeNdefMessages(QList<QNdefMessage>() << message);

    if (!id.isValid()) {
        qWarning() << "写入请求失败";
        return;
    }

    connect(target, &QNearFieldTarget::ndefMessageWritten,
            this, []() {
        qDebug() << "NDEF 消息写入成功";
    });
}
```

写入的时候一个常见的问题是标签容量不够——廉价 NFC 标签（特别是 Type 2）的可用空间可能只有几百字节，如果 NDEF 消息太大就会写入失败。写入之前最好先用 tagAccess() 或类似方法确认标签的可用空间。另外，有些 NFC 标签可能是只读的（出厂后锁定了写入权限），这种情况下 writeNdefMessages 也会失败。

### 3.5 平台支持限制

QtNfc 的平台支持情况需要认真说一下，因为这是决定"能不能用"的关键因素。

Android 是 QtNfc 支持最完善的平台。Android 系统从 4.0 开始就提供了 NFC API，QtNfc 的 Android 后端封装了这些系统 API。在 Android 上，NFC 标签检测有三种工作模式：前台调度模式（你的 App 在前台时检测标签）、读标签模式（系统级的标签读取）、Beam 模式（P2P 数据传输，Android 10 已废弃）。QtNfc 默认使用前台调度模式，即只有当你的 App 在前台时才会接收 NFC 标签事件。

iOS 的限制比较严格。Apple 只允许通过 NFCReaderSession API 读取 NDEF 标签，而且必须在 Info.plist 中添加 NFCReaderUsageDescription 权限声明。写入功能在某些 iOS 版本上可用但受限。QtNfc 对 iOS 的支持也在不断改进，但功能覆盖度不如 Android。

Linux 桌面环境基本没有 NFC 支持——虽然存在 pcsc-lite 等智能卡读取框架，但 QtNfc 没有实现 Linux 后端。Windows 情况类似。如果你需要在桌面环境做 NFC 开发，可能需要直接使用底层 SDK 或者考虑其他框架。

## 4. 综合示例：NFC 标签读写工具

把前面学的串起来，我们写一个 NFC 标签读写工具。程序启动后开始监听 NFC 标签，检测到标签后自动读取其 NDEF 消息并显示。同时提供写入功能，可以将自定义的 URI 和文本写入空白标签。完整代码见 `examples/beginner/05-other-modules/11-qtnfc-beginner/`，下面是关键部分的讲解。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Nfc Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Nfc Qt6::Widgets)
```

NFC 管理器初始化和标签检测：

```cpp
void startDetection()
{
    nfc_manager_ = new QNearFieldManager(this);

    if (!nfc_manager_->isAvailable()) {
        appendLog("本设备不支持 NFC，请使用 Android 设备测试");
        return;
    }

    connect(nfc_manager_, &QNearFieldManager::targetDetected,
            this, &NfcWindow::onTargetDetected);
    connect(nfc_manager_, &QNearFieldManager::targetLost,
            this, &NfcWindow::onTargetLost);

    nfc_manager_->setTargetAccessModes(
        QNearFieldManager::NdefRead
        | QNearFieldManager::NdefWrite);

    nfc_manager_->startTargetDetection();
    appendLog("NFC 检测已启动，请将标签靠近设备...");
}
```

检测到标签后读取 NDEF 消息：

```cpp
void onTargetDetected(QNearFieldTarget *target)
{
    appendLog("检测到标签 UID: " + target->uid().toHex());

    current_target_ = target;

    // 读取 NDEF 消息
    auto id = target->readNdefMessages();
    if (!id.isValid()) {
        appendLog("读取请求失败");
        return;
    }

    connect(target, &QNearFieldTarget::ndefMessageRead,
            this, &NfcWindow::onNdefRead);

    connect(target, &QNearFieldTarget::error,
            this, [this](QNearFieldTarget::Error error,
                         const QNearFieldTarget::RequestId &) {
        appendLog("NFC 错误: " + QString::number(
            static_cast<int>(error)));
    });
}

void onNdefRead(const QNdefMessage &message)
{
    appendLog("读取到 " + QString::number(message.size())
            + " 条 NDEF 记录");

    for (const QNdefRecord &record : message) {
        if (record.isFormatType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(record);
            appendLog("  文本: " + text.text()
                    + " (locale: " + text.locale() + ")");
        } else if (record.isFormatType<QNdefNfcUriRecord>()) {
            QNdefNfcUriRecord uri(record);
            appendLog("  URI: " + uri.uri().toString());
        } else {
            appendLog("  未知类型: "
                    + record.type().toHex());
        }
    }
}
```

写入 NDEF 消息到标签：

```cpp
void writeToTarget()
{
    if (!current_target_) {
        appendLog("没有检测到标签");
        return;
    }

    QNdefMessage message;

    QNdefNfcUriRecord uri;
    uri.setUri(QUrl(uri_edit_->text()));
    message.append(uri);

    QNdefNfcTextRecord text;
    text.setText(text_edit_->toPlainText());
    text.setLocale("en");
    message.append(text);

    auto id = current_target_->writeNdefMessages(
        QList<QNdefMessage>() << message);

    if (!id.isValid()) {
        appendLog("写入请求失败（标签可能只读或空间不足）");
        return;
    }

    connect(current_target_,
            &QNearFieldTarget::ndefMessageWritten,
            this, [this]() {
        appendLog("写入成功");
    });
}
```

运行程序后（在 Android 设备上），将一个 NFC 标签靠近手机背面，程序会自动检测标签并读取其 NDEF 内容。你可以在界面上编辑 URI 和文本，然后再次靠近标签进行写入。如果你没有 Android 设备，程序也能编译运行——只是 isAvailable() 会返回 false，日志区会提示设备不支持 NFC。

## 5. 练习项目

练习项目：NFC 智能名片。

我们要实现一个 NFC 电子名片系统。每个 NFC 标签存储一张名片信息——包含姓名、电话、邮箱。名片数据用 NDEF 文本记录存储，每条记录对应一个字段（姓名/电话/邮箱），用 locale 字段区分不同字段（比如 "name"/"phone"/"email" 作为 locale 值，虽然这不是 locale 的标准用法，但作为一个练习方案是可行的）。

完成标准是这样的：程序检测到 NFC 标签后读取所有文本记录，按 locale 字段分类显示姓名、电话、邮箱；提供写入界面，输入三个字段后写入标签；如果标签上已有名片数据，先读取再提供修改和覆盖写入的功能；支持清空标签（写入空的 NDEF 消息）。

几个实现提示：QNdefNfcTextRecord 的 locale 字段虽然是给自然语言用的，但你可以用它来标记字段类型——这在实际项目中不推荐，但作为练习足够简单；读取时遍历所有文本记录，根据 locale 把记录分配到对应的显示区域；清空标签可以写入一个包含空记录的 QNdefMessage 或者一条空的文本记录。

## 6. 官方文档参考

[Qt 文档 · QtNfc 模块](https://doc.qt.io/qt-6/qtnfc-index.html) -- NFC 模块总览

[Qt 文档 · QNearFieldManager](https://doc.qt.io/qt-6/qnearfieldmanager.html) -- NFC 标签检测管理

[Qt 文档 · QNearFieldTarget](https://doc.qt.io/qt-6/qnearfieldtarget.html) -- NFC 标签交互

[Qt 文档 · QNdefMessage](https://doc.qt.io/qt-6/qndefmessage.html) -- NDEF 消息

[Qt 文档 · QNdefNfcTextRecord](https://doc.qt.io/qt-6/qndefnfctextrecord.html) -- NDEF 文本记录

[Qt 文档 · QNdefNfcUriRecord](https://doc.qt.io/qt-6/qndefnfcurirecord.html) -- NDEF URI 记录

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。QtNfc 的 API 设计得很简洁——QNearFieldManager 负责检测标签，QNearFieldTarget 负责读写操作，QNdefMessage/QNdefRecord 负责数据封装。三个层次各司其职，上手不难。真正的门槛在平台支持——目前只有 Android 是靠谱的目标平台，iOS 受限，桌面基本不可用。如果你在做 Android 端的 NFC 应用，QtNfc 是一个不错的选择，NDEF 的读写 API 用起来很顺手。标签容量是需要时刻留意的约束——廉价 NFC 贴纸的存储空间很小，NDEF 消息太大就写不进去，开发时先用 toByteArray() 算一下序列化后的字节数再决定往标签里塞多少东西。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
