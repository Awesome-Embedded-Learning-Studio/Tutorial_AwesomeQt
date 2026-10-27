// QtWidgets 入门示例 65: QFontDialog 字体选择对话框
// 演示：getFont 模态选择
//       setCurrentFont 初始预选
//       currentFontChanged 实时预览
//       QFontDatabase 过滤等宽字体

#include "mainwindow.h"

#include <QFontDatabase>
#include <QToolBar>

// ============================================================================
// MainWindow: 演示 QFontDialog 三种用法 + 自定义等宽字体对话框
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QFontDialog 字体选择对话框演示");
    resize(700, 500);

    // ---- 中央文本编辑区 ----
    m_textEdit = new QTextEdit;
    m_textEdit->setPlainText(
        "The quick brown fox jumps over the lazy dog.\n"
        "快狐跃懒狗 —— 字体预览文字。\n\n"
        "Font Family: Microsoft YaHei\n"
        "Font Size: 12 pt\n\n"
        "int main(int argc, char *argv[])\n"
        "{\n"
        "    QApplication app(argc, argv);\n"
        "    return app.exec();\n"
        "}");

    // 设置初始字体
    QFont initialFont("Microsoft YaHei", 12);
    QFontDatabase fontDb;
    if (!fontDb.families().contains(
            initialFont.family())) {
        initialFont = QFontDatabase::systemFont(
            QFontDatabase::GeneralFont);
    }
    m_textEdit->setCurrentFont(initialFont);

    setCentralWidget(m_textEdit);

    // ---- 工具栏 ----
    auto *toolbar = addToolBar("字体操作");
    toolbar->setMovable(false);

    auto *pickAction = toolbar->addAction("选择字体");
    auto *liveAction = toolbar->addAction("实时预览");
    auto *monoAction = toolbar->addAction("等宽字体");

    connect(pickAction, &QAction::triggered,
            this, &MainWindow::onPickFont);
    connect(liveAction, &QAction::triggered,
            this, &MainWindow::onLivePreview);
    connect(monoAction, &QAction::triggered,
            this, &MainWindow::onMonoFont);
}

// ====================================================================
// 模态字体选择
// ====================================================================
void MainWindow::onPickFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(
        &ok,
        m_textEdit->currentFont(),
        this,
        "选择字体");

    if (ok) {
        m_textEdit->setCurrentFont(font);
    }
}

// ====================================================================
// 非模态实时预览
// ====================================================================
void MainWindow::onLivePreview()
{
    if (!m_fontDialog) {
        m_fontDialog = new QFontDialog(this);
        m_fontDialog->setOption(
            QFontDialog::NoButtons);
        m_fontDialog->setWindowTitle(
            "实时预览 - 选择字体");

        connect(m_fontDialog,
                &QFontDialog::currentFontChanged,
                this, [this](const QFont &font) {
                    m_textEdit->setCurrentFont(font);
                });
    }

    m_fontDialog->setCurrentFont(
        m_textEdit->currentFont());
    m_fontDialog->show();
    m_fontDialog->raise();
    m_fontDialog->activateWindow();
}

// ====================================================================
// 自定义等宽字体选择对话框
// ====================================================================
void MainWindow::onMonoFont()
{
    MonoFontDialog dialog(
        m_textEdit->currentFont(), this);

    if (dialog.exec() == QDialog::Accepted) {
        m_textEdit->setCurrentFont(
            dialog.selectedFont());
    }
}
