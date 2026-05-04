// QtWidgets 入门示例 66: QFileDialog 文件选择对话框
// 演示：getOpenFileName 打开单文件
//       getSaveFileName 保存文件
//       getOpenFileNames 批量选择
//       getExistingDirectory 选择目录
//       setNameFilter 文件类型过滤
//       QStandardPaths 默认目录

#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextStream>
#include <QToolBar>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 演示 QFileDialog 四种静态方法
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QFileDialog 文件选择对话框演示");
    resize(800, 500);

    // ---- 中央区域: 左侧文件列表 + 右侧文本编辑 ----
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_fileList = new QListWidget;
    m_fileList->setMinimumWidth(180);
    connect(m_fileList, &QListWidget::itemClicked,
            this, &MainWindow::onFileItemClicked);

    m_textEdit = new QTextEdit;
    m_textEdit->setReadOnly(true);
    m_textEdit->setPlaceholderText(
        "点击工具栏按钮打开文件...\n\n"
        "支持的操作:\n"
        "  打开文件 - 选择单个文本文件\n"
        "  批量打开 - 选择多个文本文件\n"
        "  保存文件 - 保存当前内容\n"
        "  选择目录 - 浏览目录统计");

    splitter->addWidget(m_fileList);
    splitter->addWidget(m_textEdit);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    setCentralWidget(splitter);

    // ---- 工具栏 ----
    auto *toolbar = addToolBar("文件操作");
    toolbar->setMovable(false);

    auto *openAction = toolbar->addAction("打开文件");
    auto *batchAction = toolbar->addAction("批量打开");
    toolbar->addSeparator();
    auto *saveAction = toolbar->addAction("保存文件");
    toolbar->addSeparator();
    auto *dirAction = toolbar->addAction("选择目录");

    connect(openAction, &QAction::triggered,
            this, &MainWindow::onOpenFile);
    connect(batchAction, &QAction::triggered,
            this, &MainWindow::onBatchOpen);
    connect(saveAction, &QAction::triggered,
            this, &MainWindow::onSaveFile);
    connect(dirAction, &QAction::triggered,
            this, &MainWindow::onSelectDirectory);
}

/// @brief 使用 QStandardPaths 获取合理的起始目录
QString MainWindow::defaultStartDir() const
{
    const QString docDir =
        QStandardPaths::writableLocation(
            QStandardPaths::DocumentsLocation);
    return QDir(docDir).exists() ? docDir
                                 : QDir::homePath();
}

/// @brief 读取文件内容到 m_textEdit
bool MainWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly |
                   QIODevice::Text)) {
        statusBar()->showMessage(
            "无法打开文件: " + path + " - " +
            file.errorString());
        return false;
    }

    QTextStream in(&file);
    // 自动检测 UTF-8 编码
    in.setEncoding(QStringConverter::Utf8);
    const QString content = in.readAll();
    file.close();

    m_textEdit->setPlainText(content);
    m_currentFilePath = path;

    // 窗口标题显示当前文件名
    const QString fileName =
        QFileInfo(path).fileName();
    setWindowTitle(fileName +
                   " - QFileDialog 演示");
    statusBar()->showMessage(
        "已打开: " + path, 5000);

    return true;
}

// ====================================================================
// 打开单个文件
// ====================================================================
void MainWindow::onOpenFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        "选择要打开的文本文件",
        defaultStartDir(),
        "文本文件 (*.txt);;"
        "C++ 源文件 (*.cpp *.h *.hpp);;"
        "所有文件 (*)");

    if (path.isEmpty()) {
        return;
    }

    // 清空之前的批量文件列表
    m_filePaths.clear();
    m_fileList->clear();

    loadFile(path);
}

// ====================================================================
// 批量打开多个文件
// ====================================================================
void MainWindow::onBatchOpen()
{
    const QStringList paths =
        QFileDialog::getOpenFileNames(
            this,
            "选择多个文本文件",
            defaultStartDir(),
            "文本文件 (*.txt);;"
            "C++ 源文件 (*.cpp *.h *.hpp);;"
            "所有文件 (*)");

    if (paths.isEmpty()) {
        return;
    }

    m_filePaths = paths;
    m_fileList->clear();

    for (const QString &path : paths) {
        m_fileList->addItem(
            QFileInfo(path).fileName());
    }

    // 默认显示第一个文件
    m_fileList->setCurrentRow(0);
    loadFile(paths.first());

    statusBar()->showMessage(
        QString("已加载 %1 个文件")
            .arg(paths.size()),
        5000);
}

// ====================================================================
// 点击文件列表切换显示
// ====================================================================
void MainWindow::onFileItemClicked(QListWidgetItem *item)
{
    const int row = m_fileList->row(item);
    if (row >= 0 && row < m_filePaths.size()) {
        loadFile(m_filePaths.at(row));
    }
}

// ====================================================================
// 保存文件
// ====================================================================
void MainWindow::onSaveFile()
{
    const QString dirHint =
        m_currentFilePath.isEmpty()
            ? defaultStartDir() + "/untitled.txt"
            : m_currentFilePath;

    QString path = QFileDialog::getSaveFileName(
        this,
        "保存文件",
        dirHint,
        "文本文件 (*.txt);;"
        "C++ 源文件 (*.cpp *.h);;"
        "所有文件 (*)");

    if (path.isEmpty()) {
        return;
    }

    // 自动补全 .txt 后缀（仅在用户选择文本文件过滤器时）
    if (!path.endsWith(".txt", Qt::CaseInsensitive) &&
        !path.endsWith(".cpp", Qt::CaseInsensitive) &&
        !path.endsWith(".h", Qt::CaseInsensitive)) {
        path += ".txt";
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly |
                   QIODevice::Text)) {
        QMessageBox::warning(
            this, "保存失败",
            "无法写入文件:\n" + path + "\n\n" +
                file.errorString());
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_textEdit->toPlainText();
    file.close();

    m_currentFilePath = path;
    setWindowTitle(
        QFileInfo(path).fileName() +
        " - QFileDialog 演示");
    statusBar()->showMessage(
        "已保存: " + path, 5000);
}

// ====================================================================
// 选择目录并统计
// ====================================================================
void MainWindow::onSelectDirectory()
{
    const QString dir =
        QFileDialog::getExistingDirectory(
            this,
            "选择目录",
            defaultStartDir(),
            QFileDialog::ShowDirsOnly |
            QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) {
        return;
    }

    const QDir selectedDir(dir);
    const QFileInfoList files =
        selectedDir.entryInfoList(
            QDir::Files | QDir::NoDotAndDotDot);
    const QFileInfoList subDirs =
        selectedDir.entryInfoList(
            QDir::Dirs | QDir::NoDotAndDotDot);

    const QString summary =
        QString("目录: %1\n"
                "文件数量: %2\n"
                "子目录数量: %3")
            .arg(dir)
            .arg(files.size())
            .arg(subDirs.size());

    statusBar()->showMessage(
        QString("%1 | %2 个文件, %3 个子目录")
            .arg(dir)
            .arg(files.size())
            .arg(subDirs.size()),
        10000);

    // 在文本区域显示详细内容
    QString detail = summary + "\n\n";
    detail += "--- 子目录 ---\n";
    for (const QFileInfo &d : subDirs) {
        detail += "  " + d.fileName() + "\n";
    }
    detail += "\n--- 文件 ---\n";
    for (const QFileInfo &f : files) {
        detail +=
            QString("  %1  (%2 bytes)")
                .arg(f.fileName())
                .arg(f.size()) +
            "\n";
    }

    m_textEdit->setPlainText(detail);
}
