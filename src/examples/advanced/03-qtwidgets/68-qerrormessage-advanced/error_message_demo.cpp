/// @file    error_message_demo.cpp
/// @brief   ErrorMessageDemo 类的实现。
///
/// 对应教程：进阶层 03-QtWidgets/68-QErrorMessage 进阶。

#include "error_message_demo.h"

#include <QErrorMessage>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

ErrorMessageDemo::ErrorMessageDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_settings(new QSettings("AwesomeQt", "ErrorMessageDemo", this))
{
    // @note 从 QSettings 加载之前保存的抑制列表到内存缓存，
    //       避免每次 showErrorIfNotSuppressed 都读磁盘。
    const QVariant saved = m_settings->value("suppressed_errors/ids");
    if (saved.isValid()) {
        const QStringList idList = saved.toStringList();
        m_suppressedIds = QSet<QString>(idList.begin(), idList.end());
    }

    setupUI();
    updateSuppressedCount();
}

ErrorMessageDemo::~ErrorMessageDemo()
{
    // @note 析构时将内存中的抑制集合写回 QSettings，确保数据持久化。
    QStringList idList(m_suppressedIds.begin(), m_suppressedIds.end());
    m_settings->setValue("suppressed_errors/ids", idList);
    m_settings->sync();
}

void ErrorMessageDemo::setupUI()
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);

    auto* buttonLayout = new QHBoxLayout();

    auto* fileErrorBtn = new QPushButton(tr("File Not Found Error"), this);
    auto* networkErrorBtn = new QPushButton(tr("Network Timeout Error"), this);
    auto* permissionErrorBtn = new QPushButton(tr("Permission Denied Error"), this);
    auto* resetBtn = new QPushButton(tr("Reset Suppressed Messages"), this);

    buttonLayout->addWidget(fileErrorBtn);
    buttonLayout->addWidget(networkErrorBtn);
    buttonLayout->addWidget(permissionErrorBtn);
    buttonLayout->addWidget(resetBtn);

    m_statusLabel = new QLabel(this);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addStretch();

    setCentralWidget(central);

    connect(fileErrorBtn, &QPushButton::clicked,
            this, &ErrorMessageDemo::showFileNotFoundError);
    connect(networkErrorBtn, &QPushButton::clicked,
            this, &ErrorMessageDemo::showNetworkTimeoutError);
    connect(permissionErrorBtn, &QPushButton::clicked,
            this, &ErrorMessageDemo::showPermissionDeniedError);
    connect(resetBtn, &QPushButton::clicked,
            this, &ErrorMessageDemo::resetSuppressedMessages);
}

void ErrorMessageDemo::showFileNotFoundError()
{
    showErrorIfNotSuppressed("err_file_not_found",
                             tr("File Error"),
                             tr("The requested file could not be found.\n"
                                "Please check the file path and try again."));
}

void ErrorMessageDemo::showNetworkTimeoutError()
{
    showErrorIfNotSuppressed("err_network_timeout",
                             tr("Network Error"),
                             tr("Network connection timed out.\n"
                                "The server may be unreachable."));
}

void ErrorMessageDemo::showPermissionDeniedError()
{
    showErrorIfNotSuppressed("err_permission_denied",
                             tr("Permission Error"),
                             tr("You do not have permission to perform this action.\n"
                                "Contact your administrator."));
}

void ErrorMessageDemo::resetSuppressedMessages()
{
    m_suppressedIds.clear();
    m_settings->remove("suppressed_errors/ids");
    m_settings->sync();
    updateSuppressedCount();
    m_statusLabel->setText(tr("All suppressed messages have been reset."));
}

void ErrorMessageDemo::showErrorIfNotSuppressed(const QString& messageId,
                                                const QString& title,
                                                const QString& message)
{
    if (m_suppressedIds.contains(messageId)) {
        m_statusLabel->setText(
            tr("Message \"%1\" is suppressed. Click 'Reset' to re-enable.")
                .arg(title));
        return;
    }

    // @note QErrorMessage::qtHandler() 返回全局单例，在整个应用生命周期内复用。
    QErrorMessage* errorMsg = QErrorMessage::qtHandler();
    errorMsg->setWindowTitle(title);
    errorMsg->showMessage(message);

    // @note 演示用途：弹出后立即标记为已抑制。
    // 在实际应用中，通常让用户勾选 "不再显示" 后才标记。
    markAsSuppressed(messageId);
    updateSuppressedCount();

    m_statusLabel->setText(tr("Error shown: %1").arg(title));
}

void ErrorMessageDemo::markAsSuppressed(const QString& messageId)
{
    m_suppressedIds.insert(messageId);
    // @note 每次标记都同步写入 QSettings，防止程序异常退出时丢失数据。
    QStringList idList(m_suppressedIds.begin(), m_suppressedIds.end());
    m_settings->setValue("suppressed_errors/ids", idList);
    m_settings->sync();
}

void ErrorMessageDemo::updateSuppressedCount()
{
    m_statusLabel->setText(
        tr("Suppressed messages: %1").arg(m_suppressedIds.size()));
}
