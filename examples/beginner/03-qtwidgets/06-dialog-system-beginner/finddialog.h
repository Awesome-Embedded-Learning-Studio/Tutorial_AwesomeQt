#pragma once

#include <QDialog>

class QLineEdit;

// FindDialog: 查找对话框（非模态演示）
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);

signals:
    void findRequested(const QString &text);

private:
    QLineEdit *m_findEdit = nullptr;
};
