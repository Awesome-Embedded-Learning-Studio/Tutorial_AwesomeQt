#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>

// ============================================================================
// FindDialog: 非模态对话框 — 模拟查找功能
// ============================================================================
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);

private:
    void performSearch();

    QLineEdit *m_keywordEdit = nullptr;
    QTextEdit *m_resultEdit = nullptr;
};

#endif // FINDDIALOG_H
