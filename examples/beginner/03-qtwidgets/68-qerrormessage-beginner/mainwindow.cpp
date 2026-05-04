#include "mainwindow.h"

#include <QAction>
#include <QCoreApplication>
#include <QDebug>
#include <QErrorMessage>
#include <QMetaObject>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>

// ============================================================================
// 全局指针: 供消息处理器使用的 QErrorMessage 和日志控件
// ============================================================================
static QErrorMessage *g_errorDialog = nullptr;
static QTextEdit *g_logWidget = nullptr;
static QtMessageHandler g_previousHandler = nullptr;

/// @brief 自定义全局消息处理器
static void customMessageHandler(
    QtMsgType type,
    const QMessageLogContext &context,
    const QString &msg)
{
    // 构造带前缀的日志文本
    const char *levelStr = "UNKNOWN";
    switch (type) {
    case QtDebugMsg:
        levelStr = "DEBUG";
        break;
    case QtWarningMsg:
        levelStr = "WARNING";
        break;
    case QtCriticalMsg:
        levelStr = "CRITICAL";
        break;
    case QtFatalMsg:
        levelStr = "FATAL";
        break;
    case QtInfoMsg:
        levelStr = "INFO";
        break;
    }

    const QString formatted =
        QString("[%1] %2 (%3:%4)")
            .arg(levelStr)
            .arg(msg)
            .arg(context.file ? context.file : "")
            .arg(context.line);

    // 写入 stderr（保持默认行为）
    fprintf(stderr, "%s\n", formatted.toLocal8Bit().constData());

    // 追加到日志控件（必须在主线程）
    if (g_logWidget) {
        QMetaObject::invokeMethod(
            g_logWidget,
            [formatted]() {
                g_logWidget->append(formatted);
            },
            Qt::QueuedConnection);
    }

    // qWarning 和 qCritical 弹 QErrorMessage
    if ((type == QtWarningMsg ||
         type == QtCriticalMsg) &&
        g_errorDialog) {
        QMetaObject::invokeMethod(
            g_errorDialog,
            [msg]() {
                g_errorDialog->showMessage(msg);
            },
            Qt::QueuedConnection);
    }

    // qFatal 仍然终止程序
    if (type == QtFatalMsg) {
        abort();
    }
}

// ============================================================================
// MainWindow: 演示 QErrorMessage 各种用法
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(
        "QErrorMessage 错误消息对话框演示");
    resize(700, 500);

    QCoreApplication::setOrganizationName(
        "AwesomeQt");
    QCoreApplication::setApplicationName(
        "QErrorMessageDemo");

    // ---- 中央日志区 ----
    m_logEdit = new QTextEdit;
    m_logEdit->setReadOnly(true);
    m_logEdit->setPlaceholderText(
        "日志输出区域...\n\n"
        "操作说明:\n"
        "  触发错误 - 弹出一条 QErrorMessage\n"
        "  重复错误 - 连续三次显示同一错误\n"
        "  不同类型 - 同文字不同 type 的消息\n"
        "  安装处理器 - 捕获全局 qWarning/qCritical\n"
        "  触发警告 - 通过 qWarning 触发弹窗\n"
        "  触发严重错误 - 通过 qCritical 触发弹窗\n"
        "  重置抑制列表 - 清除所有'不再显示'记录");
    g_logWidget = m_logEdit;
    setCentralWidget(m_logEdit);

    // ---- QErrorMessage 实例 ----
    ensureErrorDialog();

    // ---- 工具栏 ----
    auto *toolbar = addToolBar("操作");
    toolbar->setMovable(false);

    auto *errorAction =
        toolbar->addAction("触发错误");
    auto *repeatAction =
        toolbar->addAction("重复错误");
    auto *typedAction =
        toolbar->addAction("不同类型");

    toolbar->addSeparator();

    auto *installAction =
        toolbar->addAction("安装处理器");
    auto *warnAction =
        toolbar->addAction("触发警告");
    auto *criticalAction =
        toolbar->addAction("触发严重错误");

    toolbar->addSeparator();

    auto *resetAction =
        toolbar->addAction("重置抑制列表");

    connect(errorAction, &QAction::triggered,
            this, &MainWindow::onTriggerError);
    connect(repeatAction, &QAction::triggered,
            this, &MainWindow::onRepeatError);
    connect(typedAction, &QAction::triggered,
            this, &MainWindow::onDifferentTypes);
    connect(installAction,
            &QAction::triggered,
            this, &MainWindow::onInstallHandler);
    connect(warnAction, &QAction::triggered,
            this, &MainWindow::onTriggerWarning);
    connect(criticalAction,
            &QAction::triggered,
            this, &MainWindow::onTriggerCritical);
    connect(resetAction, &QAction::triggered,
            this, &MainWindow::onResetSuppressed);
}

MainWindow::~MainWindow()
{
    g_logWidget = nullptr;
    g_errorDialog = nullptr;
}

void MainWindow::ensureErrorDialog()
{
    if (!m_errorMsg) {
        m_errorMsg = new QErrorMessage(this);
        g_errorDialog = m_errorMsg;
    }
}

// ====================================================================
// 基本错误弹窗
// ====================================================================
void MainWindow::onTriggerError()
{
    ensureErrorDialog();
    m_errorMsg->showMessage(
        "无法加载配置文件 config.ini，"
        "将使用默认配置。");
    m_logEdit->append(
        "[操作] 触发了一条错误消息");
}

// ====================================================================
// 重复错误: 同一条消息连续三次
// 第一次弹出后勾选"不再显示"，后续两次被跳过
// ====================================================================
void MainWindow::onRepeatError()
{
    ensureErrorDialog();
    const QString msg =
        "网络连接超时，正在使用缓存数据";
    for (int i = 0; i < 3; ++i) {
        m_errorMsg->showMessage(msg);
        m_logEdit->append(
            QString("[操作] 第 %1 次调用 "
                    "showMessage(\"%2\")")
                .arg(i + 1)
                .arg(msg));
    }
}

// ====================================================================
// 不同类型: 同一文字不同 type 的消息
// ====================================================================
void MainWindow::onDifferentTypes()
{
    ensureErrorDialog();
    m_errorMsg->showMessage(
        "资源加载失败", "NetworkError");
    m_errorMsg->showMessage(
        "资源加载失败", "DiskError");
    m_logEdit->append(
        "[操作] 发送了两条文字相同但 type "
        "不同的消息");
}

// ====================================================================
// 安装全局消息处理器
// ====================================================================
void MainWindow::onInstallHandler()
{
    g_previousHandler =
        qInstallMessageHandler(
            customMessageHandler);
    m_logEdit->append(
        "[操作] 已安装自定义全局消息处理器\n"
        "  qWarning -> 写日志 + 弹窗\n"
        "  qCritical -> 写日志 + 弹窗\n"
        "  qDebug -> 只写日志");
    statusBar()->showMessage(
        "全局消息处理器已安装", 5000);
}

// ====================================================================
// 通过 qWarning 触发
// ====================================================================
void MainWindow::onTriggerWarning()
{
    qWarning() << "检测到非标准配置项:"
                << "deprecated_flag=true";
}

// ====================================================================
// 通过 qCritical 触发
// ====================================================================
void MainWindow::onTriggerCritical()
{
    qCritical() << "数据库连接失败:"
                << "Connection refused";
}

// ====================================================================
// 重置抑制列表
// ====================================================================
void MainWindow::onResetSuppressed()
{
    QSettings settings;
    settings.remove("QtErrorMessageList");
    settings.sync();

    // 重建 QErrorMessage 实例以清除内存缓存
    if (m_errorMsg) {
        m_errorMsg->deleteLater();
        m_errorMsg = nullptr;
    }
    ensureErrorDialog();

    m_logEdit->append(
        "[操作] 已清除所有'不再显示'记录"
        "并重建错误对话框实例");
    statusBar()->showMessage(
        "抑制列表已重置", 5000);
}
