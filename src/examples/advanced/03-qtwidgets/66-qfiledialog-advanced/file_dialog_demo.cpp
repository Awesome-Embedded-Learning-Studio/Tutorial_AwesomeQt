/// @file    file_dialog_demo.cpp
/// @brief   FileDialogDemo 实现——多种 QFileDialog 配置演示。
///
/// 对应教程：进阶层 03-QtWidgets/66-QFileDialog 进阶。

#include "file_dialog_demo.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

FileDialogDemo::FileDialogDemo(QWidget* parent)
    : QWidget(parent)
    , m_resultLabel(nullptr)
{
    setupUI();
}

void FileDialogDemo::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // 按钮区域
    auto* openBtn = new QPushButton(tr("Open File (with filters)"), this);
    auto* saveBtn = new QPushButton(tr("Save File..."), this);
    auto* dirBtn = new QPushButton(tr("Select Directory"), this);
    auto* qtDialogBtn = new QPushButton(tr("Open File (Qt Dialog)"), this);

    connect(openBtn, &QPushButton::clicked, this, &FileDialogDemo::openFile);
    connect(saveBtn, &QPushButton::clicked, this, &FileDialogDemo::saveFile);
    connect(dirBtn, &QPushButton::clicked, this, &FileDialogDemo::selectDirectory);
    connect(qtDialogBtn, &QPushButton::clicked, this, &FileDialogDemo::openFileQtDialog);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(openBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    auto* btnLayout2 = new QHBoxLayout;
    btnLayout2->addWidget(dirBtn);
    btnLayout2->addWidget(qtDialogBtn);
    mainLayout->addLayout(btnLayout2);

    // 结果显示
    m_resultLabel = new QLabel(tr("Result: (none)"), this);
    m_resultLabel->setWordWrap(true);
    m_resultLabel->setMinimumWidth(300);
    mainLayout->addWidget(m_resultLabel);

    setWindowTitle(tr("QFileDialog Advanced Demo"));
    resize(500, 250);
}

void FileDialogDemo::openFile()
{
    // @note 名称过滤器使用 "显示名 (glob)" 格式，多个过滤器用 ;; 分隔
    const QString filter = tr(
        "Images (*.png *.jpg *.jpeg *.bmp *.gif);;"
        "Text files (*.txt *.md *.cpp *.h);;"
        "All files (*)");

    const QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open File"), QString(), filter);

    if (!fileName.isEmpty()) {
        updateResult(tr("Opened: %1").arg(fileName));
    } else {
        updateResult(tr("Open cancelled"));
    }
}

void FileDialogDemo::saveFile()
{
    const QString filter = tr(
        "Text files (*.txt);;"
        "JSON files (*.json);;"
        "All files (*)");

    const QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save File"), QString(), filter);

    if (!fileName.isEmpty()) {
        updateResult(tr("Save to: %1").arg(fileName));
    } else {
        updateResult(tr("Save cancelled"));
    }
}

void FileDialogDemo::selectDirectory()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Directory"), QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        updateResult(tr("Directory: %1").arg(dir));
    } else {
        updateResult(tr("Directory selection cancelled"));
    }
}

void FileDialogDemo::openFileQtDialog()
{
    // @note DontUseNativeDialog 使用 Qt 内置实现，可以进行更多自定义
    // 在 Linux 上原生对话框和 Qt 对话框差异较大
    QFileDialog dialog(this, tr("Open File (Qt Built-in)"));
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setFileMode(QFileDialog::ExistingFile);

    // @note setSidebarUrls 设置侧边栏快捷访问路径
    dialog.setSidebarUrls({
        QUrl::fromLocalFile(QStringLiteral("/tmp")),
        QUrl::fromLocalFile(QDir::homePath()),
    });

    // 添加过滤器
    dialog.setNameFilter(tr("All files (*);;Text (*.txt);;C++ (*.cpp *.h)"));

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList selected = dialog.selectedFiles();
        if (!selected.isEmpty()) {
            updateResult(tr("Selected (Qt dialog): %1").arg(selected.first()));
        }
    } else {
        updateResult(tr("Qt dialog cancelled"));
    }
}

void FileDialogDemo::updateResult(const QString& text)
{
    m_resultLabel->setText(tr("Result: %1").arg(text));
}
