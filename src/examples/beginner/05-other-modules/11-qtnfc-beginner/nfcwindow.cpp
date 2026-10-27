#include "nfcwindow.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QNdefMessage>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include <QNearFieldManager>
#include <QNearFieldTarget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

// ========================================
// NFC 标签读写演示窗口
// ========================================

NfcWindow::NfcWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt NFC 标签读写工具");
    resize(600, 500);

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);

    // 顶部状态
    auto *topLayout = new QHBoxLayout();
    status_label_ = new QLabel("NFC 状态: 检查中...", this);
    start_button_ = new QPushButton("启动 NFC 检测", this);
    topLayout->addWidget(status_label_, 1);
    topLayout->addWidget(start_button_);
    mainLayout->addLayout(topLayout);

    // 写入区域
    auto *writeGroup = new QWidget(this);
    auto *writeLayout = new QVBoxLayout(writeGroup);
    writeLayout->setContentsMargins(0, 0, 0, 0);

    writeLayout->addWidget(
        new QLabel("写入内容（下次靠近标签时写入）:", this));

    auto *uriLayout = new QHBoxLayout();
    uri_layout_ = new QLineEdit(this);
    uri_layout_->setPlaceholderText("输入 URI（如 https://www.qt.io）");
    uriLayout->addWidget(new QLabel("URI:", this));
    uriLayout->addWidget(uri_layout_);
    writeLayout->addLayout(uriLayout);

    auto *textLayout = new QHBoxLayout();
    text_layout_ = new QLineEdit(this);
    text_layout_->setPlaceholderText("输入文本内容");
    textLayout->addWidget(new QLabel("文本:", this));
    textLayout->addWidget(text_layout_);
    writeLayout->addLayout(textLayout);

    write_button_ = new QPushButton("写入到标签", this);
    write_button_->setEnabled(false);
    writeLayout->addWidget(write_button_);

    mainLayout->addWidget(writeGroup);

    // 日志区
    log_text_ = new QTextEdit(this);
    log_text_->setReadOnly(true);
    log_text_->setPlaceholderText(
        "NFC 操作日志将显示在这里...\n"
        "提示：此示例需要 NFC 硬件支持（主要在 Android 上工作）");
    mainLayout->addWidget(log_text_, 1);

    // 信号连接
    connect(start_button_, &QPushButton::clicked,
            this, &NfcWindow::startDetection);
    connect(write_button_, &QPushButton::clicked,
            this, &NfcWindow::writeToTarget);

    // 初始检查 NFC 可用性
    checkNfcAvailability();
}

/// 检查设备 NFC 是否可用
void NfcWindow::checkNfcAvailability()
{
    QNearFieldManager tempManager(this);
    if (tempManager.isSupported()) {
        status_label_->setText(
            "NFC 状态: 可用 - 点击\"启动 NFC 检测\"开始");
    } else {
        status_label_->setText(
            "NFC 状态: 不可用 - "
            "此设备/平台不支持 NFC（请在 Android 设备上测试）");
        start_button_->setEnabled(false);
        appendLog("NFC 不可用。可能原因：");
        appendLog("  - 当前平台不支持 QtNfc（Linux/Windows 桌面）");
        appendLog("  - 设备没有 NFC 硬件");
        appendLog("  - Android 未声明 NFC 权限");
        appendLog("  - iOS 上 NFC 功能受限");
        appendLog("\n程序可以编译和运行，"
                 "但 NFC 功能需要实际硬件。");
    }
}

/// 启动 NFC 标签检测
void NfcWindow::startDetection()
{
    nfc_manager_ = new QNearFieldManager(this);

    if (!nfc_manager_->isSupported()) {
        appendLog("NFC 不可用，无法启动检测");
        return;
    }

    connect(nfc_manager_, &QNearFieldManager::targetDetected,
            this, &NfcWindow::onTargetDetected);
    connect(nfc_manager_, &QNearFieldManager::targetLost,
            this, &NfcWindow::onTargetLost);

    // 启动检测，支持 NDEF 访问
    nfc_manager_->startTargetDetection(
        QNearFieldTarget::NdefAccess);

    start_button_->setEnabled(false);
    status_label_->setText("NFC 状态: 检测中 - 请将标签靠近设备...");
    appendLog("NFC 检测已启动，等待标签...");
}

/// 检测到 NFC 标签
void NfcWindow::onTargetDetected(QNearFieldTarget *target)
{
    appendLog("--- 检测到 NFC 标签 ---");
    appendLog("  UID: " + target->uid().toHex());
    appendLog("  类型: " + QString::number(
        static_cast<int>(target->type())));

    current_target_ = target;
    write_button_->setEnabled(true);

    // 自动读取 NDEF 消息
    auto requestId = target->readNdefMessages();
    if (!requestId.isValid()) {
        appendLog("  NDEF 读取请求失败（标签可能为空或不支持 NDEF）");
        return;
    }

    appendLog("  正在读取 NDEF 消息...");

    // 读取成功
    connect(target, &QNearFieldTarget::ndefMessageRead,
            this, &NfcWindow::onNdefRead,
            Qt::UniqueConnection);

    // 读取/写入错误
    connect(target, &QNearFieldTarget::error,
            this, &NfcWindow::onTargetError,
            Qt::UniqueConnection);
}

/// 标签移开
void NfcWindow::onTargetLost(QNearFieldTarget *target)
{
    appendLog("--- NFC 标签已移开 ---");
    if (target == current_target_) {
        current_target_ = nullptr;
        write_button_->setEnabled(false);
    }
    target->deleteLater();
}

/// NDEF 消息读取完成
void NfcWindow::onNdefRead(const QNdefMessage &message)
{
    appendLog("  读取到 " + QString::number(message.size())
            + " 条 NDEF 记录:");

    if (message.isEmpty()) {
        appendLog("  (空标签 - 没有 NDEF 数据)");
        return;
    }

    for (int i = 0; i < message.size(); ++i) {
        const QNdefRecord &record = message.at(i);

        // 检查记录类型：NFC 文本记录 (TNF=1, type="T")
        if (record.typeNameFormat() == QNdefRecord::NfcRtd
            && record.type() == "T") {
            QNdefNfcTextRecord text(record);
            appendLog(QString("    [%1] 文本记录: \"%2\"")
                .arg(i)
                .arg(text.text()));
            appendLog(QString("        locale: %1, encoding: %2")
                .arg(text.locale())
                .arg(text.encoding()
                     == QNdefNfcTextRecord::Utf8 ? "UTF-8"
                                                  : "UTF-16"));
        } else if (record.typeNameFormat() == QNdefRecord::NfcRtd
                   && record.type() == "U") {
            // NFC URI 记录
            QNdefNfcUriRecord uri(record);
            appendLog(QString("    [%1] URI 记录: %2")
                .arg(i)
                .arg(uri.uri().toString()));
        } else {
            appendLog(QString("    [%1] 未知记录类型: %2")
                .arg(i)
                .arg(record.type().toHex()));
            appendLog(QString("        载荷 (%1 bytes): %2")
                .arg(record.payload().size())
                .arg(QString::fromUtf8(record.payload())
                     .left(80)));
        }
    }

    // 输出 NDEF 消息序列化后的大小
    QByteArray raw = message.toByteArray();
    appendLog(QString("  NDEF 总大小: %1 字节")
            .arg(raw.size()));
}

/// 写入 NDEF 消息到当前标签
void NfcWindow::writeToTarget()
{
    if (!current_target_) {
        appendLog("没有可用的标签");
        return;
    }

    QNdefMessage message;

    // 添加 URI 记录（如果有输入）
    QString uriText = uri_layout_->text().trimmed();
    if (!uriText.isEmpty()) {
        QNdefNfcUriRecord uriRecord;
        uriRecord.setUri(QUrl(uriText));
        message.append(uriRecord);
        appendLog("  构造 URI 记录: " + uriText);
    }

    // 添加文本记录（如果有输入）
    QString textContent = text_layout_->text().trimmed();
    if (!textContent.isEmpty()) {
        QNdefNfcTextRecord textRecord;
        textRecord.setText(textContent);
        textRecord.setLocale("en");
        textRecord.setEncoding(QNdefNfcTextRecord::Utf8);
        message.append(textRecord);
        appendLog("  构造文本记录: " + textContent);
    }

    if (message.isEmpty()) {
        appendLog("请至少输入 URI 或文本内容");
        return;
    }

    // 检查序列化后的大小
    QByteArray raw = message.toByteArray();
    appendLog(QString("  NDEF 消息大小: %1 字节").arg(raw.size()));

    // 写入
    auto requestId = current_target_->writeNdefMessages(
        QList<QNdefMessage>() << message);

    if (!requestId.isValid()) {
        appendLog("  写入请求失败（标签可能只读或空间不足）");
        return;
    }

    appendLog("  正在写入...");

    connect(current_target_,
            &QNearFieldTarget::requestCompleted,
            this, [this](const QNearFieldTarget::RequestId &id) {
        Q_UNUSED(id)
        appendLog("  请求已完成！");
    }, Qt::UniqueConnection);
}

/// NFC 操作错误
void NfcWindow::onTargetError(QNearFieldTarget::Error error,
                               const QNearFieldTarget::RequestId &id)
{
    Q_UNUSED(id)
    appendLog("  NFC 错误: " + QString::number(
        static_cast<int>(error)));

    // 常见错误码参考
    switch (error) {
    case QNearFieldTarget::NoError:
        break;
    case QNearFieldTarget::UnknownError:
        appendLog("    -> 未知错误");
        break;
    case QNearFieldTarget::UnsupportedError:
        appendLog("    -> 不支持的操作（标签类型不匹配）");
        break;
    case QNearFieldTarget::TargetOutOfRangeError:
        appendLog("    -> 标签超出范围");
        break;
    case QNearFieldTarget::NoResponseError:
        appendLog("    -> 标签无响应");
        break;
    case QNearFieldTarget::ChecksumMismatchError:
        appendLog("    -> 校验和不匹配");
        break;
    case QNearFieldTarget::InvalidParametersError:
        appendLog("    -> 无效参数");
        break;
    case QNearFieldTarget::ConnectionError:
        appendLog("    -> 连接错误");
        break;
    case QNearFieldTarget::NdefReadError:
        appendLog("    -> NDEF 读取错误");
        break;
    case QNearFieldTarget::NdefWriteError:
        appendLog("    -> NDEF 写入错误（标签只读或空间不足）");
        break;
    case QNearFieldTarget::CommandError:
        appendLog("    -> 命令错误");
        break;
    case QNearFieldTarget::TimeoutError:
        appendLog("    -> 超时");
        break;
    case QNearFieldTarget::UnsupportedTargetError:
        appendLog("    -> 不支持的标签");
        break;
    }
}

/// 追加日志
void NfcWindow::appendLog(const QString &text)
{
    log_text_->append(text);
    qDebug() << "[NFC]" << text;
}
