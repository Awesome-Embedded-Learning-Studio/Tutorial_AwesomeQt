/// @file    main_window.cpp
/// @brief   演示 QDialog 阻塞与非阻塞模式的主窗口类实现。
///
/// 对应教程：进阶层 03-QtWidgets/60-QDialog 异步对话框与结果回调。
/// 实现了通过 exec() 阻塞获取结果与通过 show()+accepted() 信号
/// 异步获取结果的两种对话框使用模式。

#include "main_window.h"

#include "user_input_dialog.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_logOutput(nullptr)
{
    setWindowTitle(tr("QDialog Async Demo"));
    resize(500, 400);

    auto* centralWidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    // 按钮区域
    auto* buttonLayout = new QHBoxLayout();

    auto* blockingBtn = new QPushButton(tr("Open Blocking (exec)"), this);
    auto* nonBlockingBtn = new QPushButton(tr("Open Non-Blocking (show)"), this);

    buttonLayout->addWidget(blockingBtn);
    buttonLayout->addWidget(nonBlockingBtn);
    mainLayout->addLayout(buttonLayout);

    // 日志输出区域
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setPlaceholderText(
        tr("Dialog results will appear here..."));
    mainLayout->addWidget(m_logOutput);

    setCentralWidget(centralWidget);

    // 连接按钮信号
    connect(blockingBtn, &QPushButton::clicked,
            this, &MainWindow::openBlockingDialog);
    connect(nonBlockingBtn, &QPushButton::clicked,
            this, &MainWindow::openNonBlockingDialog);

    appendLog(tr("Ready. Click a button to open a dialog."));
}

void MainWindow::openBlockingDialog()
{
    appendLog(tr("--- Blocking dialog opened ---"));

    // @note exec() 启动一个局部事件循环，阻塞直到对话框关闭。
    //       返回值是 QDialog::Accepted 或 QDialog::Rejected。
    UserInputDialog dialog(this);
    int result = dialog.exec();

    if (result == QDialog::Accepted)
    {
        appendLog(tr("[Blocking] Accepted: Name=%1, Email=%2")
                      .arg(dialog.name(), dialog.email()));
    }
    else
    {
        appendLog(tr("[Blocking] Rejected (user cancelled)."));
    }

    appendLog(tr("--- Blocking dialog closed ---"));
}

void MainWindow::openNonBlockingDialog()
{
    appendLog(tr("--- Non-blocking dialog opened ---"));
    appendLog(tr("You can still interact with this window."));

    // @note 使用 new 创建对话框，因为 show() 是非阻塞的，
    //       对话框必须存活到用户操作完成后。
    //       父对象设为 this，关闭主窗口时自动销毁对话框。
    //       设置 WA_DeleteOnClose 让对话框关闭时自动回收内存。
    auto* dialog = new UserInputDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // @note 使用 QPointer 安全地追踪对话框指针。
    //       当对话框被删除时，pointer 自动置 nullptr，
    //       避免在 onDialogAccepted 中访问已销毁的对象。
    QPointer<UserInputDialog> dialogPtr(dialog);

    connect(dialog, &QDialog::accepted, this, [this, dialogPtr]()
    {
        if (dialogPtr)
        {
            appendLog(tr("[Non-Blocking] Accepted: Name=%1, Email=%2")
                          .arg(dialogPtr->name(), dialogPtr->email()));
        }
    });

    connect(dialog, &QDialog::rejected, this, [this]()
    {
        appendLog(tr("[Non-Blocking] Rejected (user cancelled)."));
    });

    // show() 立即返回，不阻塞主窗口
    dialog->show();
}

void MainWindow::onDialogAccepted()
{
    // 此槽函数保留为信号槽风格的备选方案，
    // 实际演示中使用了 lambda 以便捕获对话框指针。
    // 详见 openNonBlockingDialog() 中的 connect 调用。
}

void MainWindow::appendLog(const QString& message)
{
    m_logOutput->append(message);
}
