#include "FindDialog.h"

// ============================================================================
// FindDialog: 非模态对话框 — 模拟查找功能
// ============================================================================
FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("查找");
    setMinimumSize(350, 200);
    // 非模态，关闭时自动销毁
    setAttribute(Qt::WA_DeleteOnClose);

    auto *layout = new QVBoxLayout(this);

    auto *searchLabel = new QLabel("搜索关键字:");
    m_keywordEdit = new QLineEdit;
    m_keywordEdit->setPlaceholderText("输入关键字...");
    layout->addWidget(searchLabel);
    layout->addWidget(m_keywordEdit);

    auto *searchBtn = new QPushButton("搜索");
    connect(searchBtn, &QPushButton::clicked,
            this, &FindDialog::performSearch);
    layout->addWidget(searchBtn);

    m_resultEdit = new QTextEdit;
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText("搜索结果将显示在这里...");
    layout->addWidget(m_resultEdit);

    // 监听关闭事件，通知外部
    connect(this, &QDialog::finished,
            this, [](int result) {
        Q_UNUSED(result);
        // 可以在这里做清理工作
    });
}

void FindDialog::performSearch()
{
    QString keyword = m_keywordEdit->text();
    if (keyword.isEmpty()) {
        m_resultEdit->setText("请输入搜索关键字");
        return;
    }
    // 模拟搜索结果
    m_resultEdit->setText(
        QString("搜索 \"%1\" 的结果:\n"
                "  - 在第 12 行找到匹配\n"
                "  - 在第 45 行找到匹配\n"
                "  - 在第 89 行找到匹配\n"
                "共找到 3 处匹配").arg(keyword));
}
